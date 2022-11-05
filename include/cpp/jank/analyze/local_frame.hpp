#pragma once

#include <clocale>
#include <fmt/core.h>
#include <unordered_map>
#include <string>
#include <functional>

#include <folly/FBVector.h>

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
  struct local_frame
  {
    enum class frame_type
    {
      root,
      fn,
      let
    };

    local_frame() = delete;
    local_frame(local_frame const &) = default;
    local_frame(local_frame &&) noexcept = default;
    local_frame(frame_type const &type, runtime::context &ctx, option<std::reference_wrapper<local_frame>> const &p)
      : type{ type }, parent{ p }, runtime_ctx{ ctx }
    { }

    local_frame& operator=(local_frame const &rhs)
    {
      if(this == &rhs)
      { return *this; }

      type = rhs.type;
      parent = rhs.parent;
      locals = rhs.locals;

      return *this;
    }
    local_frame& operator=(local_frame &&rhs)
    {
      if(this == &rhs)
      { return *this; }

      type = std::move(rhs.type);
      parent = std::move(rhs.parent);
      locals = std::move(rhs.locals);

      return *this;
    }

    /* TODO: Return the binding and all fn frames which were crossed to get it, so they can be
     * updated to register the closure. */
    option<local_binding<E>> find(runtime::obj::symbol_ptr const &sym) const
    {
      auto const result(locals.find(sym));
      if(result != locals.end())
      { return result->second; }
      else if(parent.is_some())
      { return parent.unwrap().get().find(sym); }

      return none;
    }

    struct find_result
    {
      local_binding<E> binding;
      folly::fbvector<std::reference_wrapper<local_frame>> crossed_fns;
    };

    option<find_result> find_capture(runtime::obj::symbol_ptr const &sym)
    {
      find_result acc;

      for(local_frame *it{ this }; it != nullptr; )
      {
        auto const result(it->locals.find(sym));
        if(result != it->locals.end())
        {
          acc.binding = result->second;
          break;
        }
        else if(it->parent.is_some())
        {
          if(it->type == frame_type::fn)
          { acc.crossed_fns.emplace_back(*it); }
          it = &it->parent.unwrap().get();
        }
        else
        { return none; }
      }

      return acc;
    }

    static void register_captures(find_result const &result)
    {
      for(auto &crossed_fn : result.crossed_fns)
      { crossed_fn.get().captures.emplace(result.binding.name, result.binding); }
    }

    frame_type type;
    option<std::reference_wrapper<local_frame>> parent;
    std::unordered_map<runtime::obj::symbol_ptr, local_binding<E>> locals;
    std::unordered_map<runtime::obj::symbol_ptr, local_binding<E>> captures;
    runtime::context &runtime_ctx;
  };
}
