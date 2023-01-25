#pragma once

#include <boost/variant.hpp>

#include <jank/runtime/obj/string.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression_base.hpp>

namespace jank::analyze::expr
{
  /* native/raw expressions start as a string of C++ code which can contain
   * interpolated jank code, but that string is split up into its various pieces
   * for easier codegen. */
  template <typename E>
  struct native_raw : expression_base
  {
    using chunk_t = boost::variant<native_string, native_box<E>>;

    native_vector<chunk_t> chunks;
    local_frame_ptr frame{};
  };
}
