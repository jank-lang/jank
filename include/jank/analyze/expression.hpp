#pragma once

#include <boost/variant.hpp>
#include <jank/analyze/expr/def.hpp>
#include <jank/analyze/expr/var_deref.hpp>
#include <jank/analyze/expr/call.hpp>
#include <jank/analyze/expr/primitive_literal.hpp>
#include <jank/analyze/expr/vector.hpp>
#include <jank/analyze/expr/function.hpp>
#include <jank/analyze/expr/local_reference.hpp>
#include <jank/analyze/expr/let.hpp>
#include <jank/analyze/expr/do.hpp>

namespace jank::analyze
{
  struct expression
  {
    using E = expression;
    using value_type = boost::variant
    <
      boost::recursive_wrapper<expr::def<E>>,
      expr::var_deref<E>,
      expr::call<E>,
      expr::primitive_literal<E>,
      expr::vector<E>,
      boost::recursive_wrapper<expr::function<E>>,
      expr::local_reference<E>,
      expr::let<E>,
      expr::do_<E>
    >;

    expression() = default;
    expression(expression const &) = default;
    expression(expression &&) = default;
    template <typename T>
    expression
    (
      T &&t,
      std::enable_if_t
      <
        !std::is_same_v<std::decay_t<T>, expression>
        && std::is_constructible_v<value_type, T>
      >* = nullptr
    )
      : data{ std::forward<T>(t) }
    { }

    value_type data;
  };
}
