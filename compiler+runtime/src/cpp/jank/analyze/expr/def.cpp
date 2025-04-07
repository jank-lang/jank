#include <jank/analyze/expr/def.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  def::def(expression_position const position,
           local_frame_ptr const frame,
           native_bool const needs_box,
           runtime::obj::symbol_ref const name,
           jtl::option<expression_ref> const &value)
    : expression{ expr_kind, position, frame, needs_box }
    , name{ name }
    , value{ value }
  {
  }

  object_ptr def::to_runtime_data() const
  {
    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("name"),
                                                          name,
                                                          make_box("value"),
                                                          jank::detail::to_runtime_data(value)));
  }
}
