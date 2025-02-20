#pragma once

#include <boost/variant.hpp>

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

namespace jank::analyze
{
  struct expression : gc
  {
    using E = expression;
    /* TODO: Custom variant which is more minimal. */
    using value_type = boost::variant<expr::def<E>,
                                      expr::var_deref<E>,
                                      expr::var_ref<E>,
                                      expr::call<E>,
                                      expr::primitive_literal<E>,
                                      expr::list<E>,
                                      expr::vector<E>,
                                      expr::map<E>,
                                      expr::set<E>,
                                      expr::function<E>,
                                      expr::recur<E>,
                                      expr::recursion_reference<E>,
                                      expr::named_recursion<E>,
                                      expr::local_reference,
                                      expr::let<E>,
                                      expr::letfn<E>,
                                      expr::do_<E>,
                                      expr::if_<E>,
                                      expr::throw_<E>,
                                      expr::try_<E>,
                                      expr::case_<E>>;

    static constexpr native_bool pointer_free{ false };

    expression() = default;
    expression(expression const &) = default;
    expression(expression &&) = default;

    template <typename T>
    expression(T &&t,
               std::enable_if_t<!std::is_same_v<std::decay_t<T>, expression>
                                && std::is_constructible_v<value_type, T>> * = nullptr)
      : data{ std::forward<T>(t) }
    {
    }

    expression_base_ptr get_base()
    {
      return boost::apply_visitor([](auto &typed_ex) -> expression_base_ptr { return &typed_ex; },
                                  data);
    }

    void propagate_position(expression_position const pos)
    {
      boost::apply_visitor([=](auto &typed_ex) { typed_ex.propagate_position(pos); }, data);
    }

    runtime::object_ptr to_runtime_data() const
    {
      return boost::apply_visitor([](auto &typed_ex) { return typed_ex.to_runtime_data(); }, data);
    }

    value_type data;
  };

  /* TODO: Use something non-nullable. */
  using expression_ptr = native_box<expression>;
}
