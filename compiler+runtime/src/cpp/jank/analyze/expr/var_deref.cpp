#include <jank/analyze/expr/var_deref.hpp>
#include <jank/runtime/var.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  var_deref::var_deref(expression_position const position,
                       local_frame_ptr const frame,
                       native_bool const needs_box,
                       runtime::obj::symbol_ref const qualified_name,
                       runtime::var_ref const var)
    : expression{ expr_kind, position, frame, needs_box }
    , qualified_name{ qualified_name }
    , var{ var }
  {
  }

  object_ref var_deref::to_runtime_data() const
  {
    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("qualified_name"),
                                                          qualified_name,
                                                          make_box("var"),
                                                          var));
  }
}
