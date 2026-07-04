#include <jank/runtime/ns.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/error/runtime.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime
{
  jtl::result<void, error_ref>
  ns::refer_global_impl(decltype(ns::referred_cpp_globals)::LockedPtr &locked_globals,
                        object_ref const sym)
  {
    if(sym.get_type() != object_type::symbol)
    {
      return error::runtime_invalid_referred_global_symbol(object_source(sym));
    }

    auto const sym_obj{ expect_object<obj::symbol>(sym) };

    if(!sym_obj->ns.empty())
    {
      return error::runtime_invalid_referred_global_symbol(object_source(sym_obj));
    }

    {
      if(auto const found{ (*locked_globals)->data.find(sym_obj) }; found)
      {
        return ok();
      }
    }

    auto const cpp_sym{ make_box<obj::symbol>("cpp", sym_obj->name) };
    cpp_sym->set_meta(sym_obj->get_meta());

    analyze::processor an_prc;
    auto const an_res{ an_prc.analyze(cpp_sym, analyze::expression_position::type) };
    if(an_res.is_err())
    {
      return an_res.expect_err();
    }

    auto const existing_var{ find_var(sym_obj) };
    if(existing_var.is_some())
    {
      return error::runtime_invalid_referred_global_rename(
        util::format("'{}' already refers to {} in namespace '{}'. A '{}' referred global will not "
                     "be accessible, "
                     "since vars are resolved before C++ referred globals.",
                     sym_obj->to_code_string(),
                     existing_var->to_code_string(),
                     name->to_string(),
                     sym_obj->to_code_string()),
        object_source(sym_obj));
    }

    *locked_globals = (*locked_globals)->assoc(sym_obj, sym_obj);
    return ok();
  }

  jtl::result<void, error_ref> ns::refer_global(object_ref const sym)
  {
    auto locked_globals(referred_cpp_globals.wlock());
    return refer_global_impl(locked_globals, sym);
  }

  jtl::result<void, error_ref> ns::rename_referred_globals(object_ref const rename_map)
  {
    runtime::detail::native_transient_hash_set referred_names_without_renames;
    auto locked_globals(referred_cpp_globals.wlock());

    /* We want to clear all previous renames so that the new rename map is all that we have.
     * This is important for REPL-based development, where we re-evaluated the ns form with
     * different renames. However, we want to keep :only names as they are. */
    for(auto const p : (*locked_globals)->data)
    {
      if(p.first == p.second)
      {
        referred_names_without_renames.insert(p.first);
        continue;
      }

      *locked_globals = (*locked_globals)->dissoc(p.first);
    }

    auto res{ visit_map_like(
      [&](auto const typed_rename_map) -> jtl::result<void, error_ref> {
        for(auto const p : make_sequence_range(typed_rename_map))
        {
          auto const old_name{ first(p) };
          auto const new_name{ second(p) };

          if(old_name.get_type() != object_type::symbol
             || !expect_object<obj::symbol>(old_name)->ns.empty())
          {
            return error::runtime_invalid_referred_global_symbol(object_source(old_name));
          }
          if(new_name.get_type() != object_type::symbol
             || !expect_object<obj::symbol>(new_name)->ns.empty())
          {
            return error::runtime_invalid_referred_global_symbol(object_source(new_name));
          }

          if(!referred_names_without_renames.find(old_name))
          {
            refer_global_impl(locked_globals, old_name).expect_ok();
          }

          auto const existing_var{ find_var(expect_object<obj::symbol>(new_name)) };
          if(existing_var.is_some())
          {
            return error::runtime_invalid_referred_global_rename(
              util::format(
                "'{}' already refers to {} in namespace '{}'. A '{}' referred global will not be "
                "accessible, since vars are resolved before C++ referred globals.",
                new_name.to_code_string(),
                existing_var->to_code_string(),
                name->to_string(),
                new_name.to_code_string()),
              object_source(new_name));
          }

          *locked_globals = (*locked_globals)->assoc(new_name, old_name);
        }

        return ok();
      },
      rename_map) };

    return res;
  }

}
