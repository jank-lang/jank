#pragma once

#include <unordered_map>

#include <folly/FBVector.h>

#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/option.hpp>

namespace jank::analyze
{
  struct expression;

  struct local_binding
  {
    runtime::obj::symbol_ptr name;
    option<std::reference_wrapper<expression>> value_expr;
  };

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
    local_frame
    (
      frame_type const &type,
      runtime::context &ctx,
      option<std::reference_wrapper<local_frame>> const &p
    );

    local_frame& operator=(local_frame const &rhs);
    local_frame& operator=(local_frame &&rhs);

    /* TODO: Return the binding and all fn frames which were crossed to get it, so they can be
     * updated to register the closure. */
    struct find_result
    {
      local_binding &binding;
      folly::fbvector<std::reference_wrapper<local_frame>> crossed_fns;
    };

    option<find_result> find(runtime::obj::symbol_ptr const &sym);

    static void register_captures(find_result const &result);

    frame_type type;
    option<std::reference_wrapper<local_frame>> parent;
    std::unordered_map<runtime::obj::symbol_ptr, local_binding> locals;
    std::unordered_map<runtime::obj::symbol_ptr, local_binding> captures;
    runtime::context &runtime_ctx;
  };
}
