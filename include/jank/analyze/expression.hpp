#pragma once

#include <variant>
#include <jank/analyze/expr/def.hpp>
#include <jank/analyze/expr/var_deref.hpp>
#include <jank/analyze/expr/call.hpp>
#include <jank/analyze/expr/literal.hpp>

namespace jank::analyze
{
  struct expression
  {
    using E = expression;
    using value_type = std::variant
    <
      expr::def<E>,
      expr::var_deref<E>,
      expr::call<E>,
      expr::literal<E>
    >;

    expression() = default;
    expression(expression const &) = default;
    expression(expression &&) = default;
    template <typename T>
    expression(T &&t, std::enable_if_t<!std::is_same_v<std::decay_t<T>, expression>>* = nullptr)
      : data{ t }
    { }

    value_type data;
  };
}
