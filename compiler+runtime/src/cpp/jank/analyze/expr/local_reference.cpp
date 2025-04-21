#include <jank/analyze/expr/local_reference.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  local_reference::local_reference(expression_position const position,
                                   local_frame_ptr const frame,
                                   bool const needs_box,
                                   runtime::obj::symbol_ref const name,
                                   local_binding_ptr const binding)
    : expression{ expr_kind, position, frame, needs_box }
    , name{ name }
    , binding{ binding }
  {
  }

  object_ref local_reference::to_runtime_data() const
  {
    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("name"),
                                                          name,
                                                          make_box("binding"),
                                                          binding->to_runtime_data()));
  }
}
