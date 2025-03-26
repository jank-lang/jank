#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using symbol_ptr = native_box<struct symbol>;
}

namespace jank::analyze
{
  struct processor;
  using expression_ptr = jtl::ref<struct expression>;

  namespace expr
  {
    using def_ptr = jtl::ref<struct def>;
    using var_deref_ptr = jtl::ref<struct var_deref>;
    using var_ref_ptr = jtl::ref<struct var_ref>;
    using call_ptr = jtl::ref<struct call>;
    using primitive_literal_ptr = jtl::ref<struct primitive_literal>;
    using list_ptr = jtl::ref<struct list>;
    using vector_ptr = jtl::ref<struct vector>;
    using map_ptr = jtl::ref<struct map>;
    using set_ptr = jtl::ref<struct set>;
    using local_reference_ptr = jtl::ref<struct local_reference>;
    using function_ptr = jtl::ref<struct function>;
    using recur_ptr = jtl::ref<struct recur>;
    using recursion_reference_ptr = jtl::ref<struct recursion_reference>;
    using named_recursion_ptr = jtl::ref<struct named_recursion>;
    using let_ptr = jtl::ref<struct let>;
    using do_ptr = jtl::ref<struct do_>;
    using if_ptr = jtl::ref<struct if_>;
    using throw_ptr = jtl::ref<struct throw_>;
    using try_ptr = jtl::ref<struct try_>;
    using case_ptr = jtl::ref<struct case_>;
    using function_ptr = jtl::ref<struct function>;
  }
}

namespace jank::evaluate
{
  analyze::expr::function_ptr wrap_expression(analyze::expression_ptr const expr,
                                              jtl::immutable_string const &name,
                                              native_vector<runtime::obj::symbol_ptr> params);
  analyze::expr::function_ptr wrap_expressions(native_vector<analyze::expression_ptr> const &exprs,
                                               analyze::processor const &an_prc,
                                               jtl::immutable_string const &name);

  runtime::object_ptr eval(analyze::expression_ptr);
  runtime::object_ptr eval(analyze::expr::def_ptr);
  runtime::object_ptr eval(analyze::expr::var_deref_ptr);
  runtime::object_ptr eval(analyze::expr::var_ref_ptr);
  runtime::object_ptr eval(analyze::expr::call_ptr);
  runtime::object_ptr eval(analyze::expr::primitive_literal_ptr);
  runtime::object_ptr eval(analyze::expr::list_ptr);
  runtime::object_ptr eval(analyze::expr::vector_ptr);
  runtime::object_ptr eval(analyze::expr::map_ptr);
  runtime::object_ptr eval(analyze::expr::set_ptr);
  runtime::object_ptr eval(analyze::expr::local_reference_ptr);
  runtime::object_ptr eval(analyze::expr::function_ptr);
  runtime::object_ptr eval(analyze::expr::recur_ptr);
  runtime::object_ptr eval(analyze::expr::recursion_reference_ptr);
  runtime::object_ptr eval(analyze::expr::named_recursion_ptr);
  runtime::object_ptr eval(analyze::expr::let_ptr);
  runtime::object_ptr eval(analyze::expr::do_ptr);
  runtime::object_ptr eval(analyze::expr::if_ptr);
  runtime::object_ptr eval(analyze::expr::throw_ptr);
  runtime::object_ptr eval(analyze::expr::try_ptr);
  runtime::object_ptr eval(analyze::expr::case_ptr);
}
