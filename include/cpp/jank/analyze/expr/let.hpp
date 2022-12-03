#pragma once

#include <vector>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/analyze/expr/do.hpp>
#include <jank/analyze/local_frame.hpp>

namespace jank::analyze::expr
{
  template <typename E>
  struct let
  {
    using pair_type = std::pair<runtime::obj::symbol_ptr, std::shared_ptr<E>>;

    std::vector<pair_type> pairs;
    do_<E> body;
    local_frame_ptr frame;
  };
}
