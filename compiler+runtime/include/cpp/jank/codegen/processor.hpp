#pragma once

#include <jank/analyze/processor.hpp>
#include <jank/codegen/llvm_processor.hpp>

namespace jank::analyze
{
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
    using cpp_type_ref = jtl::ref<struct cpp_type>;
    using cpp_value_ref = jtl::ref<struct cpp_value>;
    using cpp_cast_ref = jtl::ref<struct cpp_cast>;
    using cpp_call_ref = jtl::ref<struct cpp_call>;
    using cpp_constructor_call_ref = jtl::ref<struct cpp_constructor_call>;
    using cpp_member_call_ref = jtl::ref<struct cpp_member_call>;
    using cpp_member_access_ref = jtl::ref<struct cpp_member_access>;
    using cpp_builtin_operator_call_ref = jtl::ref<struct cpp_builtin_operator_call>;
    using cpp_box_ref = jtl::ref<struct cpp_box>;
    using cpp_unbox_ref = jtl::ref<struct cpp_unbox>;
  }
}

namespace jank::codegen
{
  using namespace jank::runtime;

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
    handle(jtl::immutable_string const &name, bool boxed);
    handle(jtl::immutable_string const &boxed_name);
    handle(jtl::immutable_string const &boxed_name, jtl::immutable_string const &unboxed_name);
    handle(analyze::local_binding_ptr const binding);

    handle &operator=(handle const &) = default;
    handle &operator=(handle &&) = default;

    jtl::immutable_string str(bool needs_box) const;

    jtl::immutable_string boxed_name;
    jtl::immutable_string unboxed_name;
  };

  /* Codegen processors render a single function expression to a C++ functor. REPL expressions
   * are wrapped in a nullary functor. These functors nest arbitrarily, if an expression has more
   * fn values of its own, each one rendered with its own codegen processor. */
  struct processor
  {
    processor() = delete;
    processor(analyze::expr::function_ref const expr,
              jtl::immutable_string const &module,
              compilation_target target);
    processor(processor const &) = delete;
    processor(processor &&) noexcept = delete;

    jtl::option<handle>
    gen(analyze::expression_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::def_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::var_deref_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::var_ref_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::call_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle> gen(analyze::expr::primitive_literal_ref const,
                            analyze::expr::function_arity const &,
                            bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::vector_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::map_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::set_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle> gen(analyze::expr::local_reference_ref const,
                            analyze::expr::function_arity const &,
                            bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::function_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::recur_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle> gen(analyze::expr::recursion_reference_ref const,
                            analyze::expr::function_arity const &,
                            bool box_needed);
    jtl::option<handle> gen(analyze::expr::named_recursion_ref const,
                            analyze::expr::function_arity const &,
                            bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::let_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::letfn_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::do_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::if_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::throw_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::try_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::case_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::cpp_raw_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::cpp_type_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::cpp_value_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::cpp_cast_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::cpp_call_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle> gen(analyze::expr::cpp_constructor_call_ref const,
                            analyze::expr::function_arity const &,
                            bool box_needed);
    jtl::option<handle> gen(analyze::expr::cpp_member_call_ref const,
                            analyze::expr::function_arity const &,
                            bool box_needed);
    jtl::option<handle> gen(analyze::expr::cpp_member_access_ref const,
                            analyze::expr::function_arity const &,
                            bool box_needed);
    jtl::option<handle> gen(analyze::expr::cpp_builtin_operator_call_ref const,
                            analyze::expr::function_arity const &,
                            bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::cpp_box_ref const, analyze::expr::function_arity const &, bool box_needed);
    jtl::option<handle>
    gen(analyze::expr::cpp_unbox_ref const, analyze::expr::function_arity const &, bool box_needed);

    jtl::immutable_string declaration_str();
    void build_header();
    void build_body();
    void build_footer();
    jtl::immutable_string expression_str(bool box_needed);

    jtl::immutable_string module_init_str(jtl::immutable_string const &module_name);

    void format_elided_var(jtl::immutable_string const &start,
                           jtl::immutable_string const &end,
                           jtl::immutable_string const &ret_tmp,
                           native_vector<analyze::expression_ref> const &arg_exprs,
                           analyze::expr::function_arity const &fn_arity,
                           bool arg_box_needed,
                           bool ret_box_needed);
    void format_direct_call(jtl::immutable_string const &source_tmp,
                            jtl::immutable_string const &ret_tmp,
                            native_vector<analyze::expression_ref> const &arg_exprs,
                            analyze::expr::function_arity const &fn_arity,
                            bool arg_box_needed);
    void format_dynamic_call(jtl::immutable_string const &source_tmp,
                             jtl::immutable_string const &ret_tmp,
                             native_vector<analyze::expression_ref> const &arg_exprs,
                             analyze::expr::function_arity const &fn_arity,
                             bool arg_box_needed);

    analyze::expr::function_ref root_fn;
    jtl::immutable_string module;

    compilation_target target{};
    runtime::obj::symbol struct_name;
    jtl::string_builder deps_buffer;
    jtl::string_builder header_buffer;
    jtl::string_builder body_buffer;
    jtl::string_builder footer_buffer;
    jtl::string_builder expression_buffer;
    jtl::immutable_string expression_fn_name;
    bool generated_declaration{};
    bool generated_expression{};
  };
}
