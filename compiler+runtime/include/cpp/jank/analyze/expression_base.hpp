#pragma once

#include <magic_enum.hpp>

#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze
{
  using namespace jank::runtime;

  enum class expression_position
  {
    value,
    statement,
    tail
  };

  /* Common base class for every expression. */
  struct expression_base : gc
  {
    object_ptr to_runtime_data() const
    {
      return obj::persistent_array_map::create_unique(make_box("position"),
                                                      make_box(magic_enum::enum_name(position)),
                                                      make_box("needs_box"),
                                                      make_box(needs_box));
    }

    expression_position position{};
    local_frame_ptr frame{};
    native_bool needs_box{ true };
  };

  using expression_base_ptr = native_box<expression_base>;
}
