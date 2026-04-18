#include <jank/analyze/expr/primitive_literal.hpp>
#include <jank/detail/to_runtime_data.hpp>
#include <jank/analyze/cpp_util.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  primitive_literal::primitive_literal(expression_position const position,
                                       local_frame_ptr const frame,
                                       bool const needs_box,
                                       runtime::object_ref const data)
    : expression{ expr_kind, position, frame, needs_box }
    , data{ data }
  {
  }

  object_ref primitive_literal::to_runtime_data() const
  {
    using namespace runtime::obj;

    return merge(expression::to_runtime_data(),
                 persistent_array_map::create_unique(make_box("data"), data));
  }

  jtl::ptr<void> primitive_literal::get_type() const
  {
    return cpp_util::literal_type(data);
  }
}
