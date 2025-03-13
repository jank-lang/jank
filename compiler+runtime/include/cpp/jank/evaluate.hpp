#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using symbol_ptr = native_box<struct symbol>;
}

namespace jank::analyze
{
  struct processor;
  using expression_ptr = runtime::native_box<struct expression>;

  namespace expr
  {
    using def_ptr = runtime::native_box<struct def>;
    using var_deref_ptr = runtime::native_box<struct var_deref>;
    using var_ref_ptr = runtime::native_box<struct var_ref>;
    using call_ptr = runtime::native_box<struct call>;
    using primitive_literal_ptr = runtime::native_box<struct primitive_literal>;
    using list_ptr = runtime::native_box<struct list>;
    using vector_ptr = runtime::native_box<struct vector>;
    using map_ptr = runtime::native_box<struct map>;
    using set_ptr = runtime::native_box<struct set>;
    using local_reference_ptr = runtime::native_box<struct local_reference>;
    using function_ptr = runtime::native_box<struct function>;
    using recur_ptr = runtime::native_box<struct recur>;
    using recursion_reference_ptr = runtime::native_box<struct recursion_reference>;
    using named_recursion_ptr = runtime::native_box<struct named_recursion>;
    using let_ptr = runtime::native_box<struct let>;
    using letfn_ptr = runtime::native_box<struct letfn>;
    using do_ptr = runtime::native_box<struct do_>;
    using if_ptr = runtime::native_box<struct if_>;
    using throw_ptr = runtime::native_box<struct throw_>;
    using try_ptr = runtime::native_box<struct try_>;
    using case_ptr = runtime::native_box<struct case_>;

    using function_ptr = runtime::native_box<function>;
  }
}

namespace jank::evaluate
{
  analyze::expr::function_ptr wrap_expression(analyze::expression_ptr const expr,
                                              native_persistent_string const &name,
                                              native_vector<runtime::obj::symbol_ptr> params);
  analyze::expr::function_ptr wrap_expressions(native_vector<analyze::expression_ptr> const &exprs,
                                               analyze::processor const &an_prc,
                                               native_persistent_string const &name);

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
  runtime::object_ptr eval(analyze::expr::letfn_ptr);
  runtime::object_ptr eval(analyze::expr::do_ptr);
  runtime::object_ptr eval(analyze::expr::if_ptr);
  runtime::object_ptr eval(analyze::expr::throw_ptr);
  runtime::object_ptr eval(analyze::expr::try_ptr);
  runtime::object_ptr eval(analyze::expr::case_ptr);
}
