#pragma once

#include <unordered_map>
#include <string>
#include <functional>

#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/option.hpp>

namespace jank::analyze
{
  template <typename E>
  struct local_binding
  {
    runtime::obj::symbol_ptr name;
    option<std::reference_wrapper<E>> value_expr;
  };

  template <typename E>
  struct frame
  {
    frame() = delete;
    frame(frame const &) = default;
    frame(frame &&) = default;
    frame(std::string const &label, runtime::context &ctx, option<std::reference_wrapper<frame>> const &p)
      : debug_label{ label }, parent{ p }, runtime_ctx{ ctx }
    { }

    option<local_binding<E>> find(runtime::obj::symbol_ptr const &sym) const
    {
      auto const result(locals.find(sym));
      if(result != locals.end())
      { return result->second; }
      else if(parent.is_some())
      { return parent.unwrap().get().find(sym); }

      return none;
    }

    /* TODO: Maybe remove. */
    std::string debug_label;
    option<std::reference_wrapper<frame>> parent;
    std::unordered_map<runtime::obj::symbol_ptr, local_binding<E>> locals;
    runtime::context &runtime_ctx;
  };
}
