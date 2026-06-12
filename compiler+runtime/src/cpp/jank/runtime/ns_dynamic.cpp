#include <jank/runtime/ns.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/error/runtime.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime
{
  jtl::result<void, error_ref> ns::refer_global(object_ref const sym)
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
      auto locked_globals(referred_cpp_globals.rlock());
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

    {
      auto locked_globals(referred_cpp_globals.wlock());
      *locked_globals = (*locked_globals)->assoc(sym_obj, sym_obj);
    }
    return ok();
  }
}
