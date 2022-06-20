#pragma once

#include <boost/variant.hpp>
#include <jank/analyze/expr/def.hpp>
#include <jank/analyze/expr/var_deref.hpp>
#include <jank/analyze/expr/call.hpp>
#include <jank/analyze/expr/literal.hpp>
#include <jank/analyze/expr/function.hpp>
#include <jank/analyze/expr/local_reference.hpp>

namespace jank::analyze
{
  struct expression
  {
    using E = expression;
    using value_type = boost::variant
    <
      expr::def<E>,
      expr::var_deref<E>,
      expr::call<E>,
      expr::literal<E>,
      boost::recursive_wrapper<expr::function<E>>,
      expr::local_reference<E>
    >;

    expression() = default;
    expression(expression const &) = default;
    expression(expression &&) = default;
    template <typename T>
    expression(T &&t, std::enable_if_t<!std::is_same_v<std::decay_t<T>, expression>>* = nullptr)
      : data{ std::forward<T>(t) }
    { }

    value_type data;
  };
}
