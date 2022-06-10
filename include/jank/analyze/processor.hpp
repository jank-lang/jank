#pragma once

#include <functional>

#include <jank/runtime/context.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/list.hpp>
#include <jank/analyze/expression.hpp>
#include <jank/option.hpp>

namespace jank::analyze
{
  struct local_binding
  {
    runtime::obj::symbol_ptr name;
    runtime::object_ptr value;
    expression value_expr;
  };

  /* TODO: combine this with processor? */
  struct context
  {
    context() = delete;
    context(context const &) = default;
    context(context &&) = default;
    context(runtime::context &ctx, std::string const &label, option<std::reference_wrapper<context>> const &p);

    /* TODO: Maybe remove. */
    std::string debug_label;
    option<std::reference_wrapper<context>> parent;
    std::unordered_map<runtime::obj::symbol_ptr, local_binding> locals;
    /* TODO: Only add to this during AOT. */
    //std::list<expression> exprs;
    runtime::context &runtime_ctx;
  };

  struct processor
  {
    processor() = delete;
    processor(runtime::context &rt_ctx);
    processor(processor const &) = default;
    processor(processor &&) = default;

    expression analyze(runtime::object_ptr const &o);
    expression analyze_call(runtime::obj::list_ptr const &o);

    expression analyze_def(runtime::obj::list_ptr const &);
    expression analyze_symbol(runtime::obj::symbol_ptr const &);
    expression analyze_fn(runtime::obj::list_ptr const &);
    expression analyze_let(runtime::obj::list_ptr const &);
    expression analyze_if(runtime::obj::list_ptr const &);
    expression analyze_list(runtime::obj::list_ptr const &o);

    std::unordered_map<runtime::obj::symbol_ptr, std::function<expression (runtime::obj::list_ptr const &)>> specials;
    context ctx;
  };
}
