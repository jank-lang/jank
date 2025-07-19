#include <jank/evaluate.hpp>

namespace jank::evaluate
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

  analyze::expr::function_ref wrap_expression(
    analyze::expression_ref const expr,
    jtl::immutable_string const &name,
    native_vector<runtime::obj::symbol_ref>
      params) // NOLINT(performance-unnecessary-value-param): This is a dummy implementation for static runtime
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  analyze::expr::function_ref wrap_expressions(native_vector<analyze::expression_ref> const &exprs,
                                               analyze::processor const &an_prc,
                                               jtl::immutable_string const &name)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expression_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::def_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::var_deref_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::var_ref_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::call_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::primitive_literal_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::list_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::vector_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::map_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::set_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::local_reference_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::function_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::recur_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::recursion_reference_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::named_recursion_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::let_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::letfn_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::do_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::if_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::throw_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::try_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

  runtime::object_ref eval(analyze::expr::case_ref)
  {
    throw std::runtime_error{ "Eval disabled in static runtime." };
  }

#pragma clang diagnostic pop
}
