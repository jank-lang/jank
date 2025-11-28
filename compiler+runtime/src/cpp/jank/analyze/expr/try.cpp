#include <jank/analyze/expr/try.hpp>
#include <jank/analyze/expr/do.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  try_::try_(expression_position const position,
             local_frame_ptr const frame,
             bool const needs_box,
             do_ref const body)
    : expression{ expr_kind, position, frame, needs_box }
    , body{ body }
  {
  }

  void catch_::propagate_position(expression_position const pos) const
  {
    body->propagate_position(pos);
  }

  object_ref catch_::to_runtime_data() const
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
    if(!catch_bodies.empty())
    {
      for(auto const &catch_body : catch_bodies)
      {
        catch_body.propagate_position(pos);
      }
    }
    /* The result of the 'finally' body is discarded, so we always keep it in the statement position. */
  }

  runtime::object_ref try_::to_runtime_data() const
  {
    using namespace runtime::obj;

    return runtime::merge(
      expression::to_runtime_data(),
      persistent_array_map::create_unique(make_box("body"),
                                          body->to_runtime_data(),
                                          make_box("catch"),
                                          jank::detail::to_runtime_data(catch_bodies),
                                          make_box("finally"),
                                          jank::detail::to_runtime_data(finally_body)));
  }

  void try_::walk(std::function<void(jtl::ref<expression>)> const &f)
  {
    f(body);
    if(!catch_bodies.empty())
    {
      for(auto const &catch_body : catch_bodies)
      {
        f(catch_body.body);
      }
    }
    if(finally_body.is_some())
    {
      f(finally_body.unwrap());
    }
    expression::walk(f);
  }
}
