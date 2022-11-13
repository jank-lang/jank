#pragma once

#include <memory>

#include <boost/variant.hpp>

#include <jank/analyze/expr/def.hpp>
#include <jank/analyze/expr/var_deref.hpp>
#include <jank/analyze/expr/call.hpp>
#include <jank/analyze/expr/primitive_literal.hpp>
#include <jank/analyze/expr/vector.hpp>
#include <jank/analyze/expr/map.hpp>
#include <jank/analyze/expr/function.hpp>
#include <jank/analyze/expr/local_reference.hpp>
#include <jank/analyze/expr/let.hpp>
#include <jank/analyze/expr/do.hpp>
#include <jank/analyze/expr/native_raw.hpp>

namespace jank::analyze
{
  /* TODO: Wrap all of these in shared_ptrs for reference tracking. */
  struct expression
  {
    using E = expression;
    using value_type = boost::variant
    <
      boost::recursive_wrapper<expr::def<E>>,
      expr::var_deref<E>,
      boost::recursive_wrapper<expr::call<E>>,
      expr::primitive_literal<E>,
      expr::vector<E>,
      expr::map<E>,
      boost::recursive_wrapper<expr::function<E>>,
      expr::local_reference,
      expr::let<E>,
      expr::do_<E>,
      boost::recursive_wrapper<expr::native_raw<E>>
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

    /* TODO: Keep track of references to this expression using weak_ptrs. */

    value_type data;
  };
  /* TODO: Use something non-nullable. */
  using expression_ptr = std::shared_ptr<expression>;
}
