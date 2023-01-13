#pragma once

#include <functional>

#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/list.hpp>
#include <jank/runtime/obj/vector.hpp>
#include <jank/runtime/obj/map.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression.hpp>
#include <jank/option.hpp>

namespace jank::analyze
{
  enum class source_type
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

    expression_result analyze
    (
      read::parse::processor::iterator,
      read::parse::processor::iterator const &
    );
    expression_result analyze(runtime::object_ptr, expression_type);
    expression_result analyze
    (
      runtime::object_ptr,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );
    expression_result analyze_call
    (
      runtime::obj::list_ptr const &,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );
    expression_result analyze_def
    (
      runtime::obj::list_ptr const &,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );
    expression_result analyze_symbol
    (
      runtime::obj::symbol_ptr const &,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );
    expression_result analyze_fn
    (
      runtime::obj::list_ptr const &,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );
    expression_result analyze_recur
    (
      runtime::obj::list_ptr const &,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );
    expression_result analyze_do
    (
      runtime::obj::list_ptr const &,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );
    jank::result<expr::function_arity<expression>, error> analyze_fn_arity
    (
      runtime::obj::list_ptr const &,
      local_frame_ptr &
    );
    expression_result analyze_let
    (
      runtime::obj::list_ptr const &,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );
    expression_result analyze_if
    (
      runtime::obj::list_ptr const &,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );
    expression_result analyze_quote
    (
      runtime::obj::list_ptr const &,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );
    expression_result analyze_var
    (
      runtime::obj::list_ptr const &,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );
    expression_result analyze_native_raw
    (
      runtime::obj::list_ptr const &,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );
    expression_result analyze_primitive_literal
    (
      runtime::object_ptr,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );
    expression_result analyze_vector
    (
      runtime::obj::vector_ptr const &,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );
    expression_result analyze_map
    (
      runtime::obj::map_ptr const &,
      local_frame_ptr &,
      expression_type,
      option<expr::function_context_ptr> const&
    );

    using special_function_type = std::function
    <
      expression_result
      (
        runtime::obj::list_ptr const &,
        local_frame_ptr &,
        expression_type,
        option<expr::function_context_ptr> const&
      )
    >;

    std::unordered_map<runtime::obj::symbol_ptr, special_function_type> specials;
    runtime::context &rt_ctx;
    local_frame_ptr root_frame;
  };
}
