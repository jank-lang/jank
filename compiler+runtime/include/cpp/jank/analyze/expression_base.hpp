#pragma once

#include <jank/detail/to_runtime_data.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/analyze/local_frame.hpp>

namespace jank::analyze
{
  using namespace jank::runtime;

  enum class expression_position : uint8_t
  {
    value,
    statement,
    tail
  };

  constexpr char const *expression_position_str(expression_position const pos)
  {
    switch(pos)
    {
      case expression_position::value:
        return "value";
      case expression_position::statement:
        return "statement";
      case expression_position::tail:
        return "tail";
    }
  }

  /* Common base class for every expression. */
  struct expression_base : gc
  {
    object_ptr to_runtime_data() const
    {
      return obj::persistent_array_map::create_unique(make_box("position"),
                                                      make_box(expression_position_str(position)),
                                                      make_box("needs_box"),
                                                      make_box(needs_box));
    }

    expression_position position{};
    local_frame_ptr frame{};
    native_bool needs_box{ true };
  };

  using expression_base_ptr = native_box<expression_base>;
}
