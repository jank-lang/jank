#include <jank/analyze/expr/throw.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  throw_::throw_(expression_position const position,
                 local_frame_ptr const frame,
                 bool const needs_box,
                 object_ref const form)
    : expression{ expr_kind, position, frame, needs_box, form }
  {
  }

  throw_::throw_(expression_position const position,
                 local_frame_ptr const frame,
                 bool const needs_box,
                 object_ref const form,
                 expression_ref const value)
    : expression{ expr_kind, position, frame, needs_box, form }
    , value{ value }
  {
  }

  object_ref throw_::to_runtime_data() const
  {
    object_ref v;
    if(value.is_some())
    {
      v = jank::detail::to_runtime_data(*value.unwrap());
    }
    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("value"), v));
  }

  void throw_::walk(std::function<void(jtl::ref<expression>)> const &f)
  {
    if(value.is_some())
    {
      f(value.unwrap());
    }
    expression::walk(f);
  }
}
