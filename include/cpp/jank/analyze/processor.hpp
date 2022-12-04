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

  /* Analysis contexts are meant to span the entire lifecycle of the
   * program and aid in keeping track of what's what. */
  struct context
  {
    context() = delete;
    context(runtime::context &rt_ctx);

    option<std::pair<runtime::obj::symbol_ptr, option<expression_ptr>>> find_var
    (runtime::obj::symbol_ptr const &qualified_sym) const;

    void dump() const;

    /* Generates a unique name for use with anything from codgen structs,
     * lifted vars, to shadowed locals. */
    /* TODO: Move this to RT context? */
    static runtime::obj::symbol unique_name();
    static runtime::obj::symbol unique_name(std::string_view const &prefix);

    /* These accumulate the whole lifetime of the program. */
    runtime::context &rt_ctx;
    std::unordered_map<runtime::obj::symbol_ptr, option<expression_ptr>> vars;
  };

  using error = read::error;

  struct processor
  {
    using expression_result = result<expression_ptr, error>;

    processor() = delete;
    processor
    (
      runtime::context &rt_ctx,
      read::parse::processor::iterator const &b,
      read::parse::processor::iterator const &e
    );
    processor(processor const &) = default;
    processor(processor &&) noexcept = default;

    expression_result result(context &ctx);

    expression_result analyze(runtime::object_ptr const &, context &);
    expression_result analyze(runtime::object_ptr const &, local_frame_ptr &, context &);
    expression_result analyze_call(runtime::obj::list_ptr const &, local_frame_ptr &, context &);
    expression_result analyze_def(runtime::obj::list_ptr const &, local_frame_ptr &, context &);
    expression_result analyze_symbol(runtime::obj::symbol_ptr const &, local_frame_ptr &, context &);
    expression_result analyze_fn(runtime::obj::list_ptr const &, local_frame_ptr &, context &);
    jank::result<expr::function_arity<expression>, error> analyze_fn_arity
    (runtime::obj::list_ptr const &, local_frame_ptr &, context &);
    expression_result analyze_let(runtime::obj::list_ptr const &, local_frame_ptr &, context &);
    expression_result analyze_if(runtime::obj::list_ptr const &, local_frame_ptr &, context &);
    expression_result analyze_quote(runtime::obj::list_ptr const &, local_frame_ptr &, context &);
    expression_result analyze_var(runtime::obj::list_ptr const &, local_frame_ptr &, context &);
    expression_result analyze_native_raw(runtime::obj::list_ptr const &, local_frame_ptr &, context &);
    expression_result analyze_primitive_literal(runtime::object_ptr const &, local_frame_ptr &, context &);
    expression_result analyze_vector(runtime::obj::vector_ptr const &, local_frame_ptr &, context &);
    expression_result analyze_map(runtime::obj::map_ptr const &, local_frame_ptr &, context &);

    using special_function_type = std::function
    <expression_result (runtime::obj::list_ptr const &, local_frame_ptr &, context &)>;

    std::unordered_map<runtime::obj::symbol_ptr, special_function_type> specials;
    runtime::context &rt_ctx;
    local_frame_ptr root_frame;
    read::parse::processor::iterator parse_current, parse_end;
  };
}
