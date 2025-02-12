#pragma once

#include <functional>

#include <jank/read/parse.hpp>
#include <jank/runtime/var.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression.hpp>
#include <jank/option.hpp>

namespace jank::runtime
{
  struct context;

  namespace obj
  {
    using symbol_ptr = native_box<struct symbol>;
    using persistent_list_ptr = native_box<struct persistent_list>;
    using persistent_vector_ptr = native_box<struct persistent_vector>;
    using persistent_array_map_ptr = native_box<struct persistent_array_map>;
  }
}

namespace jank::analyze
{
  enum class source_type : uint8_t
  {
    repl,
    file
  };

  using error = read::error;

  struct processor
  {
    using expression_result = result<expression_ptr, error>;

    processor() = delete;
    processor(runtime::context &rt_ctx);
    processor(processor const &) = default;
    processor(processor &&) noexcept = default;

    expression_result
    analyze(read::parse::processor::iterator, read::parse::processor::iterator const &);
    expression_result analyze(runtime::object_ptr, expression_position);
    expression_result analyze(runtime::object_ptr,
                              local_frame_ptr &,
                              expression_position,
                              option<expr::function_context_ptr> const &,
                              native_bool needs_box);
    expression_result analyze_call(runtime::obj::persistent_list_ptr const &,
                                   local_frame_ptr &,
                                   expression_position,
                                   option<expr::function_context_ptr> const &,
                                   native_bool needs_box);
    expression_result analyze_def(runtime::obj::persistent_list_ptr const &,
                                  local_frame_ptr &,
                                  expression_position,
                                  option<expr::function_context_ptr> const &,
                                  native_bool needs_box);
    expression_result analyze_symbol(runtime::obj::symbol_ptr const &,
                                     local_frame_ptr &,
                                     expression_position,
                                     option<expr::function_context_ptr> const &,
                                     native_bool needs_box);
    expression_result analyze_fn(runtime::obj::persistent_list_ptr const &,
                                 local_frame_ptr &,
                                 expression_position,
                                 option<expr::function_context_ptr> const &,
                                 native_bool needs_box);
    expression_result analyze_recur(runtime::obj::persistent_list_ptr const &,
                                    local_frame_ptr &,
                                    expression_position,
                                    option<expr::function_context_ptr> const &,
                                    native_bool needs_box);
    expression_result analyze_do(runtime::obj::persistent_list_ptr const &,
                                 local_frame_ptr &,
                                 expression_position,
                                 option<expr::function_context_ptr> const &,
                                 native_bool needs_box);
    jank::result<expr::function_arity<expression>, error>
    analyze_fn_arity(runtime::obj::persistent_list_ptr const &,
                     native_persistent_string const &name,
                     local_frame_ptr &);
    expression_result analyze_let(runtime::obj::persistent_list_ptr const &,
                                  local_frame_ptr &,
                                  expression_position,
                                  option<expr::function_context_ptr> const &,
                                  native_bool needs_box);
    expression_result analyze_letfn(runtime::obj::persistent_list_ptr const &,
                                    local_frame_ptr &,
                                    expression_position,
                                    option<expr::function_context_ptr> const &,
                                    native_bool needs_box);
    expression_result analyze_loop(runtime::obj::persistent_list_ptr const &,
                                   local_frame_ptr &,
                                   expression_position,
                                   option<expr::function_context_ptr> const &,
                                   native_bool needs_box);
    expression_result analyze_if(runtime::obj::persistent_list_ptr const &,
                                 local_frame_ptr &,
                                 expression_position,
                                 option<expr::function_context_ptr> const &,
                                 native_bool needs_box);
    expression_result analyze_quote(runtime::obj::persistent_list_ptr const &,
                                    local_frame_ptr &,
                                    expression_position,
                                    option<expr::function_context_ptr> const &,
                                    native_bool needs_box);
    expression_result analyze_var_call(runtime::obj::persistent_list_ptr const &,
                                       local_frame_ptr &,
                                       expression_position,
                                       option<expr::function_context_ptr> const &,
                                       native_bool needs_box);
    expression_result analyze_var_val(runtime::var_ptr const &,
                                      local_frame_ptr &,
                                      expression_position,
                                      option<expr::function_context_ptr> const &,
                                      native_bool needs_box);
    expression_result analyze_throw(runtime::obj::persistent_list_ptr const &,
                                    local_frame_ptr &,
                                    expression_position,
                                    option<expr::function_context_ptr> const &,
                                    native_bool needs_box);
    expression_result analyze_try(runtime::obj::persistent_list_ptr const &,
                                  local_frame_ptr &,
                                  expression_position,
                                  option<expr::function_context_ptr> const &,
                                  native_bool needs_box);
    expression_result analyze_primitive_literal(runtime::object_ptr,
                                                local_frame_ptr &,
                                                expression_position,
                                                option<expr::function_context_ptr> const &,
                                                native_bool needs_box);
    expression_result analyze_vector(runtime::obj::persistent_vector_ptr const &,
                                     local_frame_ptr &,
                                     expression_position,
                                     option<expr::function_context_ptr> const &,
                                     native_bool needs_box);
    expression_result analyze_map(object_ptr const &,
                                  local_frame_ptr &,
                                  expression_position,
                                  option<expr::function_context_ptr> const &,
                                  native_bool needs_box);
    expression_result analyze_set(object_ptr const &,
                                  local_frame_ptr &,
                                  expression_position,
                                  option<expr::function_context_ptr> const &,
                                  native_bool needs_box);

    /* Returns whether or not the form is a special symbol. */
    native_bool is_special(runtime::object_ptr form);

    using special_function_type
      = std::function<expression_result(runtime::obj::persistent_list_ptr const &,
                                        local_frame_ptr &,
                                        expression_position,
                                        option<expr::function_context_ptr> const &,
                                        native_bool)>;

    native_unordered_map<runtime::obj::symbol_ptr, special_function_type> specials;
    native_unordered_map<runtime::var_ptr, expression_ptr> vars;
    /* TODO: Remove this. */
    runtime::context &rt_ctx;
    local_frame_ptr root_frame;
  };
}
