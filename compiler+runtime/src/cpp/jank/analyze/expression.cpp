#include <jank/analyze/expression.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/analyze/visit.hpp>

namespace jank::analyze
{
  using namespace runtime;

  expression::expression(expression_kind const kind)
    : kind{ kind }
  {
  }

  expression::expression(expression_kind const kind,
                         expression_position const position,
                         local_frame_ptr const frame,
                         native_bool const needs_box)
    : kind{ kind }
    , position{ position }
    , frame{ frame }
    , needs_box{ needs_box }
  {
  }

  void expression::propagate_position(expression_position const pos)
  {
    position = pos;
  }

  object_ref expression::to_runtime_data() const
  {
    return obj::persistent_array_map::create_unique(make_box("kind"),
                                                    make_box(expression_kind_str(kind)),
                                                    make_box("position"),
                                                    make_box(expression_position_str(position)),
                                                    make_box("needs_box"),
                                                    make_box(needs_box));
  }
}
