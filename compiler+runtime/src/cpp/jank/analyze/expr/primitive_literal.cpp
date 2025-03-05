#include <jank/analyze/expr/primitive_literal.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  primitive_literal::primitive_literal(expression_position const position,
                                       local_frame_ptr const frame,
                                       native_bool const needs_box,
                                       runtime::object_ptr const data)
    : expression{ expr_kind, position, frame, needs_box }
    , data{ data }
  {
  }

  object_ptr primitive_literal::to_runtime_data() const
  {
    using namespace runtime::obj;

    return merge(expression::to_runtime_data(),
                 persistent_array_map::create_unique(make_box("data"), data));
  }
}
