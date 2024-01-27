#pragma once

#include <fmt/format.h>

#include <jank/analyze/expression.hpp>
#include <jank/analyze/processor.hpp>

namespace jank::runtime
{
  struct context;
}

namespace jank::codegen
{
  /* Each codegen operation generates its results into named C++ variables and then returns
   * an instance of this handle. Sometimes multiple variables are generated, if there's an
   * unboxed value as well as a boxed value.
   *
   * This shows up in a few interesting ways:
   *
   * 1. Let bindings with boxed and unboxed variants
   * 2. Boxed and unboxed lifted constants
   * 3. Local captures
   *
   * In the case of captures, boxing is required, but only on the captured binding and not
   * necessarily on the original. To handle this, we use the function context to denote from
   * where a binding originates.
   */
  struct handle
  {
    handle() = default;
    handle(handle const &) = default;
    handle(handle &&) = default;
    handle(native_persistent_string const &name, native_bool boxed);
    handle(native_persistent_string const &boxed_name);
    handle(native_persistent_string const &boxed_name,
           native_persistent_string const &unboxed_name);
    handle(analyze::local_binding const &binding);

    handle &operator=(handle const &) = default;
    handle &operator=(handle &&) = default;

    native_persistent_string str(native_bool needs_box) const;

    native_persistent_string boxed_name;
    native_persistent_string unboxed_name;
  };

  enum class compilation_target
  {
    ns,
    function,
    repl
  };

  /* Codegen processors render a single function expression to a C++ functor. REPL expressions
   * are wrapped in a nullary functor. These functors nest arbitrarily, if an expression has more
   * fn values of its own, each one rendered with its own codegen processor. */
  struct processor
  {
    processor() = delete;
    processor(runtime::context &rt_ctx,
              analyze::expression_ptr const &expr,
              native_persistent_string_view const &module,
              compilation_target target);
    processor(runtime::context &rt_ctx,
              analyze::expr::function<analyze::expression> const &expr,
              native_persistent_string_view const &module,
              compilation_target target);
    processor(processor const &) = delete;
    processor(processor &&) noexcept = default;

    option<handle> gen(analyze::expression_ptr const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::def<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::var_deref<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::var_ref<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::call<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::primitive_literal<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::vector<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::map<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::local_reference const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::function<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::recur<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::let<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::do_<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::if_<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::throw_<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::try_<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);
    option<handle> gen(analyze::expr::native_raw<analyze::expression> const &,
                       analyze::expr::function_arity<analyze::expression> const &,
                       native_bool box_needed);

    native_persistent_string declaration_str();
    void build_header();
    void build_body();
    void build_footer();
    native_persistent_string expression_str(native_bool box_needed);

    native_persistent_string module_init_str(native_persistent_string_view const &module);

    void format_elided_var(native_persistent_string_view const &start,
                           native_persistent_string_view const &end,
                           native_persistent_string_view const &ret_tmp,
                           native_vector<native_box<analyze::expression>> const &arg_exprs,
                           analyze::expr::function_arity<analyze::expression> const &fn_arity,
                           native_bool arg_box_needed,
                           native_bool ret_box_needed);
    void format_direct_call(native_persistent_string const &source_tmp,
                            native_persistent_string_view const &ret_tmp,
                            native_vector<native_box<analyze::expression>> const &arg_exprs,
                            analyze::expr::function_arity<analyze::expression> const &fn_arity,
                            native_bool arg_box_needed);
    void format_dynamic_call(native_persistent_string const &source_tmp,
                             native_persistent_string_view const &ret_tmp,
                             native_vector<native_box<analyze::expression>> const &arg_exprs,
                             analyze::expr::function_arity<analyze::expression> const &fn_arity,
                             native_bool arg_box_needed);

    runtime::context &rt_ctx;
    /* This is stored just to keep the expression alive. */
    analyze::expression_ptr root_expr{};
    analyze::expr::function<analyze::expression> const &root_fn;
    native_persistent_string module;

    compilation_target target{};
    runtime::obj::symbol struct_name;
    fmt::memory_buffer header_buffer;
    fmt::memory_buffer body_buffer;
    fmt::memory_buffer footer_buffer;
    fmt::memory_buffer expression_buffer;
    native_bool generated_declaration{};
    native_bool generated_expression{};
  };
}
