#include <jank/analyze/expr/cpp_call.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace Cpp
{
  std::string GetName(void *);
}

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  cpp_call::cpp_call()
    : expression{ expr_kind }
  {
  }

  cpp_call::cpp_call(expression_position const position,
                     local_frame_ptr const frame,
                     bool const needs_box,
                     jtl::ptr<void> const type,
                     jtl::ptr<void> const fn,
                     native_vector<expression_ref> &&arg_exprs)
    : expression{ expr_kind, position, frame, needs_box }
    , type{ type }
    , fn{ fn }
    , arg_exprs{ jtl::move(arg_exprs) }
  {
  }

  void cpp_call::propagate_position(expression_position const pos)
  {
    position = pos;
  }

  object_ref cpp_call::to_runtime_data() const
  {
    auto arg_expr_maps(make_box<obj::persistent_vector>());
    for(auto const &e : arg_exprs)
    {
      arg_expr_maps = arg_expr_maps->conj(e->to_runtime_data());
    }

    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("fn"),
                                                          make_box(Cpp::GetName(fn)),
                                                          make_box("arg_exprs"),
                                                          arg_expr_maps));
  }
}
