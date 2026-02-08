#include <jank/runtime/ns.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/error/report.hpp>
#include <jank/error/runtime.hpp>

namespace jank::runtime
{
  ns::ns(obj::symbol_ref const name)
    : object{ obj_type, obj_behaviors }
    , name{ name }
    , vars{ obj::persistent_hash_map::empty() }
    , aliases{ obj::persistent_hash_map::empty() }
    , referred_cpp_globals{ obj::persistent_hash_map::empty() }
  {
  }

  var_ref ns::intern_var(jtl::immutable_string_view const &name)
  {
    return intern_var(make_box<obj::symbol>(name));
  }

  var_ref ns::intern_var(obj::symbol_ref const sym)
  {
    obj::symbol_ref unqualified_sym{ sym };
    if(!unqualified_sym->ns.empty())
    {
      unqualified_sym = make_box<obj::symbol>("", sym->name);
    }

    /* TODO: Read lock, then upgrade as needed? Benchmark. */
    auto locked_vars(vars.wlock());
    object_ref const * const found_var((*locked_vars)->data.find(unqualified_sym));
    if(found_var && found_var->is_some())
    {
      /* TODO: Why not store var_ref instead? Relying on expect_object is not good. */
      return expect_object<var>(*found_var);
    }

    auto const new_var(make_box<var>(this, unqualified_sym));
    *locked_vars
      = make_box<obj::persistent_hash_map>((*locked_vars)->data.set(unqualified_sym, new_var));
    return new_var;
  }

  var_ref ns::intern_owned_var(jtl::immutable_string_view const &name)
  {
    return intern_owned_var(make_box<obj::symbol>(name));
  }

  /* Interning an owned var is different from just interning a var, since we will replace
   * any existing referred var with our own. This leads to a warning. For example:
   *
   * ```
   * user=> (ns foo)
   * nil
   *
   * foo=> (defn println [])
   * WARNING 'println' already referred to #'clojure.core/println in namespace 'foo' but has been replaced by #'foo/println
   * ```
   */
  var_ref ns::intern_owned_var(obj::symbol_ref const sym)
  {
    obj::symbol_ref unqualified_sym{ sym };
    if(!unqualified_sym->ns.empty())
    {
      unqualified_sym = make_box<obj::symbol>("", sym->name);
    }

    /* TODO: Read lock, then upgrade as needed? Benchmark. */
    auto locked_vars(vars.wlock());
    object_ref const * const found_var((*locked_vars)->data.find(unqualified_sym));
    bool redefined{};
    if(found_var && found_var->is_some())
    {
      auto const v{ expect_object<var>(*found_var) };
      if(v->n == this)
      {
        return v;
      }

      redefined = true;
    }

    auto const new_var(make_box<var>(this, unqualified_sym));
    if(redefined)
    {
      auto const v{ expect_object<var>(*found_var) };
      error::warn(
        util::format("'{}' already referred to {} in namespace '{}' but has been replaced by {}",
                     unqualified_sym->to_string(),
                     v->to_code_string(),
                     name->to_string(),
                     new_var->to_code_string()));
    }
    *locked_vars
      = make_box<obj::persistent_hash_map>((*locked_vars)->data.set(unqualified_sym, new_var));
    return new_var;
  }

  jtl::result<void, jtl::immutable_string> ns::unmap(obj::symbol_ref const sym)
  {
    if(!sym->ns.empty())
    {
      return err(util::format("Can't unintern namespace-qualified symbol: {}", sym->to_string()));
    }

    auto locked_vars(vars.wlock());
    *locked_vars = make_box<obj::persistent_hash_map>((*locked_vars)->data.erase(sym));
    return ok();
  }

  var_ref ns::find_var(obj::symbol_ref const sym)
  {
    if(!sym->ns.empty())
    {
      return __rt_ctx->find_var(sym);
    }

    auto const locked_vars(vars.rlock());
    auto const found((*locked_vars)->data.find(sym));
    if(!found)
    {
      return {};
    }

    return { expect_object<var>(*found) };
  }

  jtl::result<void, jtl::immutable_string>
  ns::add_alias(obj::symbol_ref const sym, ns_ref const nsp)
  {
    auto locked_aliases(aliases.wlock());
    auto const found((*locked_aliases)->data.find(sym));
    if(found)
    {
      auto const existing(expect_object<ns>(*found));
      if(existing != nsp)
      {
        return err(util::format("{} already aliases {} in ns {}, cannot change to {}",
                                sym->to_string(),
                                existing->to_string(),
                                to_string(),
                                nsp->to_string()));
      }
      return ok();
    }
    *locked_aliases = make_box<obj::persistent_hash_map>((*locked_aliases)->data.set(sym, nsp));
    return ok();
  }

  void ns::remove_alias(obj::symbol_ref const sym)
  {
    auto locked_aliases(aliases.wlock());
    *locked_aliases = make_box<obj::persistent_hash_map>((*locked_aliases)->data.erase(sym));
  }

  ns_ref ns::find_alias(obj::symbol_ref const sym) const
  {
    auto locked_aliases(aliases.rlock());
    auto const found((*locked_aliases)->data.find(sym));
    if(found)
    {
      return expect_object<ns>(*found);
    }
    return {};
  }

  jtl::result<void, jtl::immutable_string> ns::refer(obj::symbol_ref const sym, var_ref const var)
  {
    auto locked_vars(vars.wlock());
    if(auto const found = (*locked_vars)->data.find(sym))
    {
      if(found->data->type == object_type::var)
      {
        auto const found_var(expect_object<runtime::var>(found->data));
        auto const clojure_core(__rt_ctx->find_ns(make_box<obj::symbol>("clojure.core")));
        if(var->n != found_var->n && (found_var->n != clojure_core))
        {
          return err(util::format("{} already refers to {} in ns {}",
                                  sym->to_string(),
                                  expect_object<runtime::var>(*found)->to_string(),
                                  to_string()));
        }
      }
    }
    *locked_vars = make_box<obj::persistent_hash_map>((*locked_vars)->data.set(sym, var));
    return ok();
  }

  jtl::result<void, error_ref> ns::refer_global(object_ref const sym)
  {
    if(sym->type != object_type::symbol)
    {
      return error::runtime_invalid_referred_global_symbol(object_source(sym));
    }

    auto const sym_obj{ expect_object<obj::symbol>(sym) };

    if(!sym_obj->ns.empty())
    {
      return error::runtime_invalid_referred_global_symbol(object_source(sym_obj));
    }

    {
      auto locked_globals(referred_cpp_globals.rlock());
      if(auto const found{ (*locked_globals)->data.find(sym_obj) }; found)
      {
        return ok();
      }
    }

    auto const cpp_sym{ make_box<obj::symbol>("cpp", sym_obj->name) };
    cpp_sym->meta = sym_obj->meta;

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

    {
      auto locked_globals(referred_cpp_globals.wlock());
      *locked_globals = (*locked_globals)->assoc(sym_obj, sym_obj);
    }
    return ok();
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

          if(old_name->type != object_type::symbol
             || !expect_object<obj::symbol>(old_name)->ns.empty())
          {
            return error::runtime_invalid_referred_global_symbol(object_source(old_name));
          }
          if(new_name->type != object_type::symbol
             || !expect_object<obj::symbol>(new_name)->ns.empty())
          {
            return error::runtime_invalid_referred_global_symbol(object_source(new_name));
          }

          if(!referred_names_without_renames.find(old_name))
          {
            return error::runtime_invalid_referred_global_rename(
              "Symbols in :rename must also be referred in :only.",
              object_source(old_name));
          }

          auto const existing_var{ find_var(expect_object<obj::symbol>(new_name)) };
          if(existing_var.is_some())
          {
            return error::runtime_invalid_referred_global_rename(
              util::format(
                "'{}' already refers to {} in namespace '{}'. A '{}' referred global will not be "
                "accessible, since vars are resolved before C++ referred globals.",
                new_name->to_code_string(),
                existing_var->to_code_string(),
                name->to_string(),
                new_name->to_code_string()),
              object_source(new_name));
          }

          *locked_globals = (*locked_globals)->assoc(new_name, old_name);
        }

        return ok();
      },
      rename_map) };

    return res;
  }

  obj::symbol_ref ns::find_referred_global(obj::symbol_ref const sym)
  {
    auto locked_globals(referred_cpp_globals.rlock());
    auto const found{ (*locked_globals)->data.find(sym) };
    return found ? try_object<obj::symbol>(*found) : obj::symbol_ref{};
  }

  obj::persistent_hash_map_ref ns::get_mappings() const
  {
    auto const locked_vars(vars.rlock());
    return *locked_vars;
  }

  bool ns::equal(object const &o) const
  {
    if(o.type != object_type::ns)
    {
      return false;
    }

    auto const v(expect_object<ns>(&o));
    return name == v->name;
  }
  jtl::immutable_string ns::to_string() const
  /* TODO: Maybe cache this. */
  {
    return name->to_string();
  }

  jtl::immutable_string ns::to_code_string() const
  {
    return to_string();
  }

  void ns::to_string(jtl::string_builder &buff) const
  {
    name->to_string(buff);
  }

  object_ref ns::with_meta(object_ref const)
  {
    throw std::runtime_error{
      "Value of type 'ns' does not support 'with-meta'.",
    };
  }

  object_ref ns::get_meta() const
  {
    return meta;
  }

  bool ns::operator==(ns const &rhs) const
  {
    return name == rhs.name;
  }
}
