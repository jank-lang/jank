#pragma once

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/expr/do.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/detail/to_runtime_data.hpp>
#include <jank/runtime/seq.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct let : expression_base
  {
    using pair_type = std::pair<runtime::obj::symbol_ptr, native_box<E>>;

    let(expression_type const type, bool const needs_box, local_frame_ptr const f)
      : expression_base{ gc{}, type, f, needs_box }
    { }
    native_vector<pair_type> pairs;
    do_<E> body;

    runtime::object_ptr to_runtime_data() const
    {
      runtime::object_ptr pair_maps(make_box<runtime::obj::vector>());
      for(auto const &e : pairs)
      {
        pair_maps = runtime::conj
        (
          pair_maps,
          make_box<runtime::obj::vector>
          (
            detail::to_runtime_data(frame->find_local_or_capture(e.first).unwrap().binding),
            e.second->to_runtime_data()
          )
        );
      }

      return runtime::obj::map::create_unique
      (
        make_box("__type"), make_box("expr::let"),
        make_box("pairs"), pair_maps,
        make_box("body"), detail::to_runtime_data(body)
      );
    }
  };
}
