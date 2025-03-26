#pragma once

#include <jtl/option.hpp>

#include <jank/result.hpp>
#include <jank/analyze/expression.hpp>

namespace jank::analyze
{
  namespace expr
  {
    struct def;
    struct var_deref;
    struct var_ref;
    struct call;
    struct primitive_literal;
    struct list;
    struct vector;
    struct map;
    struct set;
    struct function;
    struct recur;
    struct recursion_reference;
    struct named_recursion;
    struct local_reference;
    struct let;
    struct do_;
    struct if_;
    struct throw_;
    struct try_;
  }

  enum class visit_step
  {
    pre,
    post
  };

  enum class visit_action
  {
    proceed,
    skip,
    stop
  };

  template <typename R, typename E>
  struct recursive_visitor
  {
    using result_type = result<R, E>;

    struct context
    {
      visit_step step{};
      visit_action action{ visit_action::proceed };
      [[no_unique_address]]
      std::conditional_t<std::is_same_v<R, void>, none_t, std::vector<R>> results;
    };

    virtual ~recursive_visitor() = default;

    result_type visit(expression_ptr expr);

    virtual result_type visit(expr::def const &expr);
    virtual result_type visit(expr::var_deref const &expr);
    virtual result_type visit(expr::var_ref const &expr);
    virtual result_type visit(expr::call const &expr);
    virtual result_type visit(expr::primitive_literal const &expr);
    virtual result_type visit(expr::list const &expr);
    virtual result_type visit(expr::vector const &expr);
    virtual result_type visit(expr::map const &expr);
    virtual result_type visit(expr::set const &expr);
    virtual result_type visit(expr::function const &expr);
    virtual result_type visit(expr::recur const &expr);
    virtual result_type visit(expr::recursion_reference const &expr);
    virtual result_type visit(expr::named_recursion const &expr);
    virtual result_type visit(expr::local_reference const &expr);
    virtual result_type visit(expr::let const &expr);
    virtual result_type visit(expr::do_ const &expr);
    virtual result_type visit(expr::if_ const &expr);
    virtual result_type visit(expr::throw_ const &expr);
    virtual result_type visit(expr::try_ const &expr);

    context ctx;
  };
}
