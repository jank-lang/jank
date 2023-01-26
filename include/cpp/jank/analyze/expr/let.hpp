#pragma once

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/expr/do.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct let : expression_base
  {
    using pair_type = std::pair<runtime::obj::symbol_ptr, native_box<E>>;

    let(expression_type const type, local_frame_ptr const f)
      : expression_base{ gc{}, type }, frame{ f }
    { }
    native_vector<pair_type> pairs;
    do_<E> body;
    local_frame_ptr frame{};
  };
}
