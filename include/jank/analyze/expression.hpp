#pragma once

#include <variant>
#include <jank/analyze/expr/def.hpp>
#include <jank/analyze/expr/var_deref.hpp>
#include <jank/analyze/expr/call.hpp>
#include <jank/analyze/expr/list.hpp>

namespace jank::analyze
{
  struct expression
  {
    using E = expression;
    using value_type = std::variant<expr::def<E>, expr::var_deref<E>, expr::call<E>, expr::list<E>>;

    expression() = default;
    template <typename T>
    expression(T &&t)
      : data{ std::forward<T>(t) }
    { }

    value_type data;
  };
}
