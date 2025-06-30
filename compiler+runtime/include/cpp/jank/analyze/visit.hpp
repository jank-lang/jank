#pragma once

#include <jank/analyze/expression.hpp>
#include <jank/analyze/expr/def.hpp>
#include <jank/analyze/expr/var_deref.hpp>
#include <jank/analyze/expr/var_ref.hpp>
#include <jank/analyze/expr/call.hpp>
#include <jank/analyze/expr/primitive_literal.hpp>
#include <jank/analyze/expr/list.hpp>
#include <jank/analyze/expr/vector.hpp>
#include <jank/analyze/expr/map.hpp>
#include <jank/analyze/expr/set.hpp>
#include <jank/analyze/expr/function.hpp>
#include <jank/analyze/expr/recur.hpp>
#include <jank/analyze/expr/recursion_reference.hpp>
#include <jank/analyze/expr/named_recursion.hpp>
#include <jank/analyze/expr/local_reference.hpp>
#include <jank/analyze/expr/let.hpp>
#include <jank/analyze/expr/letfn.hpp>
#include <jank/analyze/expr/do.hpp>
#include <jank/analyze/expr/if.hpp>
#include <jank/analyze/expr/throw.hpp>
#include <jank/analyze/expr/try.hpp>
#include <jank/analyze/expr/case.hpp>
#include <jank/analyze/expr/cpp_raw.hpp>
#include <jank/analyze/expr/cpp_type.hpp>
#include <jank/analyze/expr/cpp_value.hpp>
#include <jank/analyze/expr/cpp_cast.hpp>
#include <jank/analyze/expr/cpp_call.hpp>
#include <jank/analyze/expr/cpp_constructor_call.hpp>
#include <jank/analyze/expr/cpp_member_call.hpp>
#include <jank/analyze/expr/cpp_member_access.hpp>
#include <jank/analyze/expr/cpp_builtin_operator_call.hpp>
#include <jank/analyze/expr/cpp_box.hpp>
#include <jank/analyze/expr/cpp_unbox.hpp>
#include <jank/analyze/expr/cpp_new.hpp>
#include <jank/analyze/expr/cpp_delete.hpp>

namespace jank::analyze
{
  template <typename F, typename... Args>
  auto visit_expr(F const &f, expression_ref const e, Args &&...args)
  {
    switch(e->kind)
    {
      case expression_kind::def:
        return f(jtl::static_ref_cast<expr::def>(e), std::forward<Args>(args)...);
      case expression_kind::var_deref:
        return f(jtl::static_ref_cast<expr::var_deref>(e), std::forward<Args>(args)...);
      case expression_kind::var_ref:
        return f(jtl::static_ref_cast<expr::var_ref>(e), std::forward<Args>(args)...);
      case expression_kind::call:
        return f(jtl::static_ref_cast<expr::call>(e), std::forward<Args>(args)...);
      case expression_kind::primitive_literal:
        return f(jtl::static_ref_cast<expr::primitive_literal>(e), std::forward<Args>(args)...);
      case expression_kind::list:
        return f(jtl::static_ref_cast<expr::list>(e), std::forward<Args>(args)...);
      case expression_kind::vector:
        return f(jtl::static_ref_cast<expr::vector>(e), std::forward<Args>(args)...);
      case expression_kind::map:
        return f(jtl::static_ref_cast<expr::map>(e), std::forward<Args>(args)...);
      case expression_kind::set:
        return f(jtl::static_ref_cast<expr::set>(e), std::forward<Args>(args)...);
      case expression_kind::function:
        return f(jtl::static_ref_cast<expr::function>(e), std::forward<Args>(args)...);
      case expression_kind::recur:
        return f(jtl::static_ref_cast<expr::recur>(e), std::forward<Args>(args)...);
      case expression_kind::recursion_reference:
        return f(jtl::static_ref_cast<expr::recursion_reference>(e), std::forward<Args>(args)...);
      case expression_kind::named_recursion:
        return f(jtl::static_ref_cast<expr::named_recursion>(e), std::forward<Args>(args)...);
      case expression_kind::local_reference:
        return f(jtl::static_ref_cast<expr::local_reference>(e), std::forward<Args>(args)...);
      case expression_kind::let:
        return f(jtl::static_ref_cast<expr::let>(e), std::forward<Args>(args)...);
      case expression_kind::letfn:
        return f(jtl::static_ref_cast<expr::letfn>(e), std::forward<Args>(args)...);
      case expression_kind::do_:
        return f(jtl::static_ref_cast<expr::do_>(e), std::forward<Args>(args)...);
      case expression_kind::if_:
        return f(jtl::static_ref_cast<expr::if_>(e), std::forward<Args>(args)...);
      case expression_kind::throw_:
        return f(jtl::static_ref_cast<expr::throw_>(e), std::forward<Args>(args)...);
      case expression_kind::try_:
        return f(jtl::static_ref_cast<expr::try_>(e), std::forward<Args>(args)...);
      case expression_kind::case_:
        return f(jtl::static_ref_cast<expr::case_>(e), std::forward<Args>(args)...);
      case expression_kind::cpp_raw:
        return f(jtl::static_ref_cast<expr::cpp_raw>(e), std::forward<Args>(args)...);
      case expression_kind::cpp_type:
        return f(jtl::static_ref_cast<expr::cpp_type>(e), std::forward<Args>(args)...);
      case expression_kind::cpp_value:
        return f(jtl::static_ref_cast<expr::cpp_value>(e), std::forward<Args>(args)...);
      case expression_kind::cpp_cast:
        return f(jtl::static_ref_cast<expr::cpp_cast>(e), std::forward<Args>(args)...);
      case expression_kind::cpp_call:
        return f(jtl::static_ref_cast<expr::cpp_call>(e), std::forward<Args>(args)...);
      case expression_kind::cpp_constructor_call:
        return f(jtl::static_ref_cast<expr::cpp_constructor_call>(e), std::forward<Args>(args)...);
      case expression_kind::cpp_member_call:
        return f(jtl::static_ref_cast<expr::cpp_member_call>(e), std::forward<Args>(args)...);
      case expression_kind::cpp_member_access:
        return f(jtl::static_ref_cast<expr::cpp_member_access>(e), std::forward<Args>(args)...);
      case expression_kind::cpp_builtin_operator_call:
        return f(jtl::static_ref_cast<expr::cpp_builtin_operator_call>(e),
                 std::forward<Args>(args)...);
      case expression_kind::cpp_box:
        return f(jtl::static_ref_cast<expr::cpp_box>(e), std::forward<Args>(args)...);
      case expression_kind::cpp_unbox:
        return f(jtl::static_ref_cast<expr::cpp_unbox>(e), std::forward<Args>(args)...);
      case expression_kind::cpp_new:
        return f(jtl::static_ref_cast<expr::cpp_new>(e), std::forward<Args>(args)...);
      case expression_kind::cpp_delete:
        return f(jtl::static_ref_cast<expr::cpp_delete>(e), std::forward<Args>(args)...);

      case expression_kind::uninitialized:
        break;
    }
    throw std::runtime_error{ "Invalid expression kind "
                              + std::to_string(static_cast<int>(e->kind)) };
  }
}
