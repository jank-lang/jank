#pragma once

#include <functional>

#include <jank/runtime/context.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/list.hpp>
#include <jank/runtime/obj/vector.hpp>
#include <jank/analyze/frame.hpp>
#include <jank/analyze/expression.hpp>
#include <jank/option.hpp>

namespace jank::analyze
{
  struct processor
  {
    processor() = delete;
    processor(runtime::context &rt_ctx);
    processor(processor const &) = default;
    processor(processor &&) = default;

    expression analyze(runtime::object_ptr const &);
    expression analyze(runtime::object_ptr const &, frame<expression> &);
    expression analyze_call(runtime::obj::list_ptr const &, frame<expression> &);

    expression analyze_def(runtime::obj::list_ptr const &, frame<expression> &);
    expression analyze_symbol(runtime::obj::symbol_ptr const &, frame<expression> &);
    expression analyze_fn(runtime::obj::list_ptr const &, frame<expression> &);
    expression analyze_let(runtime::obj::list_ptr const &, frame<expression> &);
    expression analyze_if(runtime::obj::list_ptr const &, frame<expression> &);
    expression analyze_quote(runtime::obj::list_ptr const &, frame<expression> &);
    expression analyze_primitive_literal(runtime::object_ptr const &, frame<expression> &);
    expression analyze_vector(runtime::obj::vector_ptr const &, frame<expression> &);

    using special_function_type = std::function<expression (runtime::obj::list_ptr const &, frame<expression> &)>;
    std::unordered_map<runtime::obj::symbol_ptr, special_function_type> specials;
    frame<expression> root_frame;
  };
}
