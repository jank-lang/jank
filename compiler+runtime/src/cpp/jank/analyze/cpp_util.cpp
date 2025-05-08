#include <algorithm>

#include <clang/Interpreter/CppInterOp.h>
#include <Interpreter/Compatibility.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Sema/Sema.h>

#include <jank/analyze/cpp_util.hpp>
#include <jank/analyze/visit.hpp>
#include <jank/runtime/context.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/scope_exit.hpp>
#include <jank/error/analyze.hpp>

namespace jank::analyze::cpp_util
{
  jtl::ptr<void> resolve_type(jtl::immutable_string const &sym)
  {
    return Cpp::GetType(sym);
  }

  /* Resolves the specified dot-separated symbol into its scope.
   *
   * For example, `std.string.iterator` gives us the scope for iterator in std::string.
   *
   * This doesn't work on built-in types, such as `int`, since they don't have a scope. */
  jtl::string_result<jtl::ptr<void>> resolve_scope(jtl::immutable_string const &sym)
  {
    jtl::ptr<void> scope{ Cpp::GetGlobalScope() };
    usize new_start{};
    while(true)
    {
      auto const dot{ sym.find('.', new_start) };
      if(dot == jtl::immutable_string::npos)
      {
        /* Finding dots will still leave us with the last part of the symbol to lookup. */
        scope = Cpp::GetNamed(sym.substr(new_start), scope);
        break;
      }
      auto const subs{ sym.substr(new_start, dot - new_start) };
      new_start = dot + 1;
      scope = Cpp::GetNamed(subs, scope);
      if(!scope)
      {
        return err(util::format("Unable to find scope for symbol '{}'.", sym));
      }
    }

    if(scope)
    {
      return ok(scope);
    }
    return err(util::format("Unable to find scope for symbol '{}'.", sym));
  }

  jtl::ptr<void> untyped_object_ptr_type()
  {
    static jtl::ptr<void> const ret{ Cpp::GetPointerType(Cpp::GetTypeFromScope(
      Cpp::GetNamed("object", Cpp::GetNamed("runtime", Cpp::GetNamed("jank"))))) };
    return ret;
  }

  jtl::ptr<void> untyped_object_ref_type()
  {
    static jtl::ptr<void> const ret{ Cpp::GetCanonicalType(
      Cpp::GetTypeFromScope(Cpp::GetScopeFromCompleteName("jank::runtime::object_ref"))) };
    return ret;
  }

  bool is_untyped_object(jtl::ptr<void> const type)
  {
    auto const can_type{ Cpp::GetCanonicalType(type) };
    return can_type == untyped_object_ptr_type() || can_type == untyped_object_ref_type();
  }

  static jtl::ptr<void> oref_template()
  {
    static jtl::ptr<void> const ret{ Cpp::GetUnderlyingScope(
      Cpp::GetScopeFromCompleteName("jank::runtime::oref")) };
    return ret;
  }

  /* TODO: Support for typed object raw pointers. */
  bool is_typed_object(jtl::ptr<void> const type)
  {
    auto const can_type{ Cpp::GetCanonicalType(type) };
    /* TODO: Need underlying? */
    auto const scope{ Cpp::GetUnderlyingScope(Cpp::GetScopeFromType(can_type)) };
    return !is_untyped_object(can_type) && scope
      && Cpp::IsTemplateSpecializationOf(scope, oref_template());
  }

  bool is_any_object(jtl::ptr<void> type)
  {
    return is_untyped_object(type) || is_typed_object(type);
  }

  jtl::ptr<void> expression_type(expression_ref const expr)
  {
    return visit_expr(
      [](auto const typed_expr) -> jtl::ptr<void> {
        using T = typename decltype(typed_expr)::value_type;

        if constexpr(jtl::is_any_same<T,
                                      expr::cpp_type,
                                      expr::cpp_value,
                                      expr::cpp_cast,
                                      expr::cpp_call,
                                      expr::cpp_constructor_call>)
        {
          return typed_expr->type;
        }
        else
        {
          return untyped_object_ptr_type();
        }
      },
      expr);
  }

  jtl::string_result<std::vector<Cpp::TemplateArgInfo>>
  find_best_arg_types_with_conversions(std::vector<void *> const &fns,
                                       std::vector<Cpp::TemplateArgInfo> const &args)
  {
    auto const arg_count{ args.size() };
    usize max_arg_count{};
    std::vector<void *> matching_fns;

    /* First rule out any fns not matching the arity we've specified. However, note that C++
     * fns can have default arguments which needn't be specified. */
    for(auto const fn : fns)
    {
      auto const num_args{ Cpp::GetFunctionNumArgs(fn) };
      if(Cpp::GetFunctionRequiredArgs(fn) <= arg_count && arg_count <= num_args)
      {
        matching_fns.emplace_back(fn);
        max_arg_count = std::max<usize>(max_arg_count, num_args);
      }
    }

    std::vector<Cpp::TemplateArgInfo> converted_args{ args };

    /* If any arg can be implicitly converted to multiple functions, we have an ambiguity.
     * The user will need to specify the correct type by using a cast. */
    for(usize arg_idx{}; arg_idx < max_arg_count; ++arg_idx)
    {
      /* TODO: Check for typed and untyped objects. */

      /* If our input argument here isn't an object ptr, there's no implicit conversion
       * we're going to consider. Skip to the next argument. */
      auto const is_untyped_obj{ is_untyped_object(args[arg_idx].m_Type) };
      /* TODO: Check the other way, too, if we're calling a fn taking objects. */
      if(!is_untyped_obj)
      {
        continue;
      }

      jtl::option<usize> needed_conversion;
      for(usize fn_idx{}; fn_idx < fns.size(); ++fn_idx)
      {
        auto const param_type{ Cpp::GetFunctionArgType(fns[fn_idx], arg_idx) };
        if(!param_type)
        {
          continue;
        }
        if(Cpp::IsImplicitlyConvertible(args[arg_idx].m_Type, param_type))
        {
          continue;
        }

        if(is_convertible(param_type))
        {
          if(needed_conversion.is_some())
          {
            return err("");
          }
          needed_conversion = fn_idx;
          converted_args[arg_idx] = param_type;
        }
      }
    }

    return ok(std::move(converted_args));
  }

  /* TODO: Cache result. */
  bool is_convertible(jtl::ptr<void> const type)
  {
    static auto const convert_template{ Cpp::GetScopeFromCompleteName("jank::runtime::convert") };
    Cpp::TemplateArgInfo arg{ Cpp::GetTypeWithoutCv(
      Cpp::GetNonReferenceType(Cpp::GetCanonicalType(type))) };
    clang::Sema::SFINAETrap trap{ runtime::__rt_ctx->jit_prc.interpreter->getSema() };
    Cpp::TCppScope_t instantiation{};
    {
      auto &diag{ runtime::__rt_ctx->jit_prc.interpreter->getCompilerInstance()->getDiagnostics() };
      auto old_client{ diag.takeClient() };
      diag.setClient(new clang::IgnoringDiagConsumer{}, true);
      util::scope_exit const finally{ [&] { diag.setClient(old_client.release(), true); } };
      instantiation = Cpp::InstantiateTemplate(convert_template, &arg, 1);
    }
    return !trap.hasErrorOccurred() && Cpp::IsComplete(instantiation);
  }

  usize offset_to_typed_object_base(jtl::ptr<void> const type)
  {
    jank_debug_assert(is_typed_object(type));
    auto const can_type{ Cpp::GetCanonicalType(type) };
    auto const scope{ Cpp::GetUnderlyingScope(
      Cpp::GetNamed("value_type", Cpp::GetScopeFromType(can_type))) };
    jank_debug_assert(scope);
    auto const base{ Cpp::LookupDatamember("base", scope) };
    jank_debug_assert(base);
    auto const offset{ Cpp::GetVariableOffset(base, scope) };
    return offset;
  }

  jtl::result<void, error_ref> ensure_convertible(expression_ref const expr)
  {
    auto const type{ expression_type(expr) };
    if(!is_any_object(type) && !is_convertible(type))
    {
      return error::analyze_invalid_conversion(
        util::format("This function is returning a native object of type '{}', which is not "
                     "convertible to a jank runtime object.",
                     Cpp::GetTypeAsString(type)));
    }
    return ok();
  }
}
