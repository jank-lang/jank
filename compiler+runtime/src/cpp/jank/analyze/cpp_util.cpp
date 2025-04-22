#include <clang/Interpreter/CppInterOp.h>

#include <algorithm>
#include <jank/analyze/cpp_util.hpp>
#include <jank/analyze/visit.hpp>
#include <jank/util/fmt/print.hpp>

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
        return err(util::format("Unable to find scope for symbol '{}'", sym));
      }
    }

    if(scope)
    {
      return ok(scope);
    }
    return err(util::format("Unable to find scope for symbol '{}'", sym));
  }

  static jtl::ptr<void> object_ptr_type()
  {
    static jtl::ptr<void> const obj_ptr_type{ (
      Cpp::Declare("namespace jank::runtime{ struct object; }"),
      Cpp::GetPointerType(Cpp::GetTypeFromScope(
        Cpp::GetNamed("object", Cpp::GetNamed("runtime", Cpp::GetNamed("jank")))))) };
    return obj_ptr_type;
  }

  jtl::ptr<void> expression_type(expression_ref const expr)
  {
    return visit_expr(
      [](auto const typed_expr) -> jtl::ptr<void> {
        using T = typename decltype(typed_expr)::value_type;

        if constexpr(std::same_as<T, expr::cpp_type> || std::same_as<T, expr::cpp_value>)
        {
          return typed_expr->type;
        }
        else
        {
          return object_ptr_type();
        }
      },
      expr);
  }

  jtl::ptr<void> find_best_overload_with_conversions(std::vector<void *> const &fns,
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

    auto const obj_ptr_type{ object_ptr_type() };
    auto const obj_ptr_type_str{ Cpp::GetTypeAsString(obj_ptr_type) };

    /* If any arg can be implicitly converted to multiple functions, we have an ambiguity.
     * The user will need to specify the correct type by using a cast. */
    for(usize arg_idx{}; arg_idx < max_arg_count; ++arg_idx)
    {
      /* TODO: Is this how types should be compared? */

      /* If our input argument here isn't an object ptr, there's no implicit conversion
       * we're going to consider. Skip to the next argument. */
      auto const is_arg_object_ptr{ Cpp::GetTypeAsString(args[arg_idx].m_Type)
                                    == obj_ptr_type_str };
      if(!is_arg_object_ptr)
      {
        continue;
      }

      std::vector<void *> arg_types;
      for(auto const fn : fns)
      {
        auto const fn_arg_type{ Cpp::GetFunctionArgType(fn, arg_idx) };
        arg_types.emplace_back(fn_arg_type);
      }

      /* TODO: Check for possible conversion. If the conversion is possible
       * in multiple fns, we may have an ambiguity. Need to check arity after that, to
       * resolve? */
    }

    return nullptr;
  }
}
