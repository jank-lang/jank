#include <jank/analyze/expr/var_ref.hpp>
#include <jank/detail/to_runtime_data.hpp>
#include <jank/runtime/var.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  var_ref::var_ref(expression_position const position,
                   local_frame_ptr const frame,
                   native_bool const needs_box,
                   runtime::obj::symbol_ptr const qualified_name,
                   runtime::var_ptr const var)
    : expression{ expr_kind, position, frame, needs_box }
    , qualified_name{ qualified_name }
    , var{ var }
  {
  }

  object_ptr var_ref::to_runtime_data() const
  {
    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("qualified_name"),
                                                          qualified_name,
                                                          make_box("var"),
                                                          var));
  }
}
