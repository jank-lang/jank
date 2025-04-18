#pragma once

#include <functional>

#include <jtl/option.hpp>

#include <jank/read/parse.hpp>
#include <jank/runtime/var.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression.hpp>
#include <jank/analyze/expr/function.hpp>

namespace jank::runtime
{
  struct context;

  namespace obj
  {
    using symbol_ref = oref<struct symbol>;
    using persistent_list_ref = oref<struct persistent_list>;
    using persistent_vector_ref = oref<struct persistent_vector>;
    using persistent_array_map_ref = oref<struct persistent_array_map>;
  }
}

namespace jank::analyze
{
  enum class source_type : u8
  {
    repl,
    file
  };

  struct processor
  {
    using expression_result = jtl::result<expression_ref, error_ref>;

    processor() = delete;
    processor(runtime::context &rt_ctx);
    processor(processor const &) = default;
    processor(processor &&) noexcept = default;

    expression_result
    analyze(read::parse::processor::iterator, read::parse::processor::iterator const &);
    expression_result analyze(runtime::object_ref, expression_position);
    expression_result analyze(runtime::object_ref,
                              local_frame_ptr,
                              expression_position,
                              jtl::option<expr::function_context_ref> const &,
                              bool needs_box);
    expression_result analyze_call(runtime::obj::persistent_list_ref const,
                                   local_frame_ptr,
                                   expression_position,
                                   jtl::option<expr::function_context_ref> const &,
                                   bool needs_box);
    expression_result analyze_def(runtime::obj::persistent_list_ref const,
                                  local_frame_ptr,
                                  expression_position,
                                  jtl::option<expr::function_context_ref> const &,
                                  bool needs_box);
    expression_result analyze_symbol(runtime::obj::symbol_ref const,
                                     local_frame_ptr,
                                     expression_position,
                                     jtl::option<expr::function_context_ref> const &,
                                     bool needs_box);
    expression_result analyze_fn(runtime::obj::persistent_list_ref const,
                                 local_frame_ptr,
                                 expression_position,
                                 jtl::option<expr::function_context_ref> const &,
                                 bool needs_box);
    expression_result analyze_recur(runtime::obj::persistent_list_ref const,
                                    local_frame_ptr,
                                    expression_position,
                                    jtl::option<expr::function_context_ref> const &,
                                    bool needs_box);
    expression_result analyze_do(runtime::obj::persistent_list_ref const,
                                 local_frame_ptr,
                                 expression_position,
                                 jtl::option<expr::function_context_ref> const &,
                                 bool needs_box);
    jtl::result<expr::function_arity, error_ref>
    analyze_fn_arity(runtime::obj::persistent_list_ref const,
                     jtl::immutable_string const &name,
                     local_frame_ptr);
    expression_result analyze_let(runtime::obj::persistent_list_ref const,
                                  local_frame_ptr,
                                  expression_position,
                                  jtl::option<expr::function_context_ref> const &,
                                  bool needs_box);
    expression_result analyze_letfn(runtime::obj::persistent_list_ref const &,
                                    local_frame_ptr,
                                    expression_position,
                                    jtl::option<expr::function_context_ref> const &,
                                    bool needs_box);
    expression_result analyze_loop(runtime::obj::persistent_list_ref const,
                                   local_frame_ptr,
                                   expression_position,
                                   jtl::option<expr::function_context_ref> const &,
                                   bool needs_box);
    expression_result analyze_if(runtime::obj::persistent_list_ref const,
                                 local_frame_ptr,
                                 expression_position,
                                 jtl::option<expr::function_context_ref> const &,
                                 bool needs_box);
    expression_result analyze_quote(runtime::obj::persistent_list_ref const,
                                    local_frame_ptr,
                                    expression_position,
                                    jtl::option<expr::function_context_ref> const &,
                                    bool needs_box);
    expression_result analyze_var_call(runtime::obj::persistent_list_ref const,
                                       local_frame_ptr,
                                       expression_position,
                                       jtl::option<expr::function_context_ref> const &,
                                       bool needs_box);
    expression_result analyze_var_val(runtime::var_ref const,
                                      local_frame_ptr,
                                      expression_position,
                                      jtl::option<expr::function_context_ref> const &,
                                      bool needs_box);
    expression_result analyze_throw(runtime::obj::persistent_list_ref const,
                                    local_frame_ptr,
                                    expression_position,
                                    jtl::option<expr::function_context_ref> const &,
                                    bool needs_box);
    expression_result analyze_try(runtime::obj::persistent_list_ref const,
                                  local_frame_ptr,
                                  expression_position,
                                  jtl::option<expr::function_context_ref> const &,
                                  bool needs_box);
    expression_result analyze_primitive_literal(runtime::object_ref,
                                                local_frame_ptr,
                                                expression_position,
                                                jtl::option<expr::function_context_ref> const &,
                                                bool needs_box);
    expression_result analyze_vector(runtime::obj::persistent_vector_ref const,
                                     local_frame_ptr,
                                     expression_position,
                                     jtl::option<expr::function_context_ref> const &,
                                     bool needs_box);
    expression_result analyze_map(runtime::object_ref const,
                                  local_frame_ptr,
                                  expression_position,
                                  jtl::option<expr::function_context_ref> const &,
                                  bool needs_box);
    expression_result analyze_set(runtime::object_ref const,
                                  local_frame_ptr,
                                  expression_position,
                                  jtl::option<expr::function_context_ref> const &,
                                  bool needs_box);

    expression_result analyze_case(runtime::obj::persistent_list_ref const,
                                   local_frame_ptr,
                                   expression_position,
                                   jtl::option<expr::function_context_ref> const &,
                                   bool needs_box);

    /* Returns whether the form is a special symbol. */
    bool is_special(runtime::object_ref form);

    using special_function_type
      = std::function<expression_result(runtime::obj::persistent_list_ref const,
                                        local_frame_ptr,
                                        expression_position,
                                        jtl::option<expr::function_context_ref> const &,
                                        bool)>;

    native_unordered_map<runtime::obj::symbol_ref, special_function_type> specials;
    native_unordered_map<runtime::var_ref, expression_ref> vars;
    /* TODO: Remove this. */
    runtime::context &rt_ctx;
    local_frame_ptr root_frame;
    native_vector<runtime::object_ref> macro_expansions;
  };
}
