#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using symbol_ref = oref<struct symbol>;
}

namespace jank::analyze
{
  struct processor;
  using expression_ref = jtl::ref<struct expression>;

  namespace expr
  {
    using def_ref = jtl::ref<struct def>;
    using var_deref_ref = jtl::ref<struct var_deref>;
    using var_ref_ref = jtl::ref<struct var_ref>;
    using call_ref = jtl::ref<struct call>;
    using primitive_literal_ref = jtl::ref<struct primitive_literal>;
    using list_ref = jtl::ref<struct list>;
    using vector_ref = jtl::ref<struct vector>;
    using map_ref = jtl::ref<struct map>;
    using set_ref = jtl::ref<struct set>;
    using local_reference_ref = jtl::ref<struct local_reference>;
    using function_ref = jtl::ref<struct function>;
    using recur_ref = jtl::ref<struct recur>;
    using recursion_reference_ref = jtl::ref<struct recursion_reference>;
    using named_recursion_ref = jtl::ref<struct named_recursion>;
    using let_ref = jtl::ref<struct let>;
    using letfn_ref = jtl::ref<struct letfn>;
    using do_ref = jtl::ref<struct do_>;
    using if_ref = jtl::ref<struct if_>;
    using throw_ref = jtl::ref<struct throw_>;
    using try_ref = jtl::ref<struct try_>;
    using case_ref = jtl::ref<struct case_>;
    using function_ref = jtl::ref<struct function>;
  }
}

namespace jank::evaluate
{
  analyze::expr::function_ref wrap_expression(analyze::expression_ref const expr,
                                              jtl::immutable_string const &name,
                                              native_vector<runtime::obj::symbol_ref> params);
  analyze::expr::function_ref wrap_expressions(native_vector<analyze::expression_ref> const &exprs,
                                               analyze::processor const &an_prc,
                                               jtl::immutable_string const &name);

  runtime::object_ref eval(analyze::expression_ref);
  runtime::object_ref eval(analyze::expr::def_ref);
  runtime::object_ref eval(analyze::expr::var_deref_ref);
  runtime::object_ref eval(analyze::expr::var_ref_ref);
  runtime::object_ref eval(analyze::expr::call_ref);
  runtime::object_ref eval(analyze::expr::primitive_literal_ref);
  runtime::object_ref eval(analyze::expr::list_ref);
  runtime::object_ref eval(analyze::expr::vector_ref);
  runtime::object_ref eval(analyze::expr::map_ref);
  runtime::object_ref eval(analyze::expr::set_ref);
  runtime::object_ref eval(analyze::expr::local_reference_ref);
  runtime::object_ref eval(analyze::expr::function_ref);
  runtime::object_ref eval(analyze::expr::recur_ref);
  runtime::object_ref eval(analyze::expr::recursion_reference_ref);
  runtime::object_ref eval(analyze::expr::named_recursion_ref);
  runtime::object_ref eval(analyze::expr::let_ref);
  runtime::object_ref eval(analyze::expr::letfn_ref);
  runtime::object_ref eval(analyze::expr::do_ref);
  runtime::object_ref eval(analyze::expr::if_ref);
  runtime::object_ref eval(analyze::expr::throw_ref);
  runtime::object_ref eval(analyze::expr::try_ref);
  runtime::object_ref eval(analyze::expr::case_ref);
}
