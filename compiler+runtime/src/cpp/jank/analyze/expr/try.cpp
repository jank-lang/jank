#include <jank/analyze/expr/try.hpp>
#include <jank/analyze/expr/do.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  try_::try_(expression_position const position,
             local_frame_ptr const frame,
             native_bool const needs_box,
             do_ref const body)
    : expression{ expr_kind, position, frame, needs_box }
    , body{ body }
  {
  }

  void catch_::propagate_position(expression_position const pos) const
  {
    body->propagate_position(pos);
  }

  object_ptr catch_::to_runtime_data() const
  {
    using namespace runtime::obj;

    return persistent_array_map::create_unique(make_box("body"),
                                               body->to_runtime_data(),
                                               make_box("sym"),
                                               sym);
  }

  void try_::propagate_position(expression_position const pos)
  {
    position = pos;
    body->propagate_position(pos);
    if(catch_body)
    {
      catch_body.unwrap().propagate_position(pos);
    }
    if(finally_body)
    {
      finally_body.unwrap()->propagate_position(pos);
    }
  }

  runtime::object_ptr try_::to_runtime_data() const
  {
    using namespace runtime::obj;

    return runtime::merge(
      expression::to_runtime_data(),
      persistent_array_map::create_unique(make_box("body"),
                                          body->to_runtime_data(),
                                          make_box("catch"),
                                          jank::detail::to_runtime_data(catch_body),
                                          make_box("finally"),
                                          jank::detail::to_runtime_data(finally_body)));
  }
}
