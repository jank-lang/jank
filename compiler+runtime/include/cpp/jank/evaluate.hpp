#pragma once

#include <jank/analyze/expression.hpp>

namespace jank::analyze
{
  struct processor;
}

namespace jank::evaluate
{
  analyze::expression_ptr wrap_expression(analyze::expression_ptr const expr,
                                          native_persistent_string const &name,
                                          native_vector<analyze::obj::symbol_ptr> params);
  analyze::expression_ptr wrap_expressions(native_vector<analyze::expression_ptr> const &exprs,
                                           analyze::processor const &an_prc,
                                           native_persistent_string const &name);

  runtime::object_ptr eval(analyze::expression_ptr const &);
  runtime::object_ptr eval(analyze::expr::def<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::var_deref<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::var_ref<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::call<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::primitive_literal<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::list<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::vector<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::map<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::set<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::local_reference const &);
  runtime::object_ptr eval(analyze::expr::function<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::recur<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::recursion_reference<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::named_recursion<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::let<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::letfn<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::do_<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::if_<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::throw_<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::try_<analyze::expression> const &);
  runtime::object_ptr eval(analyze::expr::case_<analyze::expression> const &);
}
