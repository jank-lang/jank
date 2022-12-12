#pragma once

#include <jank/runtime/object.hpp>
#include <jank/analyze/expression.hpp>

namespace jank::evaluate
{
  runtime::object_ptr eval(runtime::context &, jit::processor const &, analyze::expression_ptr const &);
  runtime::object_ptr eval(runtime::context &, jit::processor const &, analyze::expr::def<analyze::expression> const &);
  runtime::object_ptr eval(runtime::context &, jit::processor const &, analyze::expr::var_deref<analyze::expression> const &);
  runtime::object_ptr eval(runtime::context &, jit::processor const &, analyze::expr::var_ref<analyze::expression> const &);
  runtime::object_ptr eval(runtime::context &, jit::processor const &, analyze::expr::call<analyze::expression> const &);
  runtime::object_ptr eval(runtime::context &, jit::processor const &, analyze::expr::primitive_literal<analyze::expression> const &);
  runtime::object_ptr eval(runtime::context &, jit::processor const &, analyze::expr::vector<analyze::expression> const &);
  runtime::object_ptr eval(runtime::context &, jit::processor const &, analyze::expr::map<analyze::expression> const &);
  runtime::object_ptr eval(runtime::context &, jit::processor const &, analyze::expr::local_reference const &);
  runtime::object_ptr eval(runtime::context &, jit::processor const &, analyze::expr::function<analyze::expression> const &);
  runtime::object_ptr eval(runtime::context &, jit::processor const &, analyze::expr::let<analyze::expression> const &);
  runtime::object_ptr eval(runtime::context &, jit::processor const &, analyze::expr::do_<analyze::expression> const &);
  runtime::object_ptr eval(runtime::context &, jit::processor const &, analyze::expr::if_<analyze::expression> const &);
  runtime::object_ptr eval(runtime::context &, jit::processor const &, analyze::expr::native_raw<analyze::expression> const &);
}
