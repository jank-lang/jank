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
  struct lifted_var
  {
    runtime::obj::symbol local_name;
    runtime::obj::symbol_ptr var_name;
  };

  struct lifted_constant
  {
    runtime::obj::symbol local_name;
    runtime::object_ptr data;
  };

  struct context
  {
    context() = delete;
    context(runtime::context &rt_ctx);

    runtime::obj::symbol_ptr lift_var(runtime::obj::symbol_ptr const &);
    option<std::reference_wrapper<lifted_var>> find_lifted_var(runtime::obj::symbol_ptr const &);
    void lift_constant(runtime::object_ptr const &);
    option<std::reference_wrapper<lifted_constant>> find_lifted_constant(runtime::object_ptr const &);

    static runtime::obj::symbol unique_name();
    static runtime::obj::symbol unique_name(std::string const &prefix);

    runtime::context &rt_ctx;
    std::unordered_map<runtime::obj::symbol_ptr, lifted_var> lifted_vars;
    std::unordered_map<runtime::object_ptr, lifted_constant> lifted_constants;
  };

  struct processor
  {
    processor() = delete;
    processor(runtime::context &rt_ctx);
    processor(processor const &) = default;
    processor(processor &&) = default;

    expression analyze(runtime::object_ptr const &, context &);
    expression analyze(runtime::object_ptr const &, frame<expression> &, context &);
    expression analyze_call(runtime::obj::list_ptr const &, frame<expression> &, context &);
    expression analyze_def(runtime::obj::list_ptr const &, frame<expression> &, context &);
    expression analyze_symbol(runtime::obj::symbol_ptr const &, frame<expression> &, context &);
    expression analyze_fn(runtime::obj::list_ptr const &, frame<expression> &, context &);
    expression analyze_let(runtime::obj::list_ptr const &, frame<expression> &, context &);
    expression analyze_if(runtime::obj::list_ptr const &, frame<expression> &, context &);
    expression analyze_quote(runtime::obj::list_ptr const &, frame<expression> &, context &);
    expression analyze_primitive_literal(runtime::object_ptr const &, frame<expression> &, context &);
    expression analyze_vector(runtime::obj::vector_ptr const &, frame<expression> &, context &);

    using special_function_type = std::function<expression (runtime::obj::list_ptr const &, frame<expression> &, context &)>;
    std::unordered_map<runtime::obj::symbol_ptr, special_function_type> specials;
    runtime::context &rt_ctx;
    frame<expression> root_frame;
  };
}
