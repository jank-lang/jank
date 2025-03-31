#include <jank/analyze/expr/list.hpp>
#include <jank/detail/to_runtime_data.hpp>

namespace jank::analyze::expr
{
  using namespace jank::runtime;

  list::list(expression_position const position,
             local_frame_ptr const frame,
             native_bool const needs_box,
             native_vector<expression_ref> &&data_exprs,
             jtl::option<runtime::object_ptr> const &meta)
    : expression{ expr_kind, position, frame, needs_box }
    , data_exprs{ std::move(data_exprs) }
    , meta{ meta }
  {
  }

  object_ptr list::to_runtime_data() const
  {
    auto exprs(make_box<obj::persistent_vector>());
    for(auto const &e : data_exprs)
    {
      exprs = exprs->conj(e->to_runtime_data());
    }

    return merge(expression::to_runtime_data(),
                 obj::persistent_array_map::create_unique(make_box("data_exprs"),
                                                          exprs,
                                                          make_box("meta"),
                                                          jank::detail::to_runtime_data(meta)));
  }
}
