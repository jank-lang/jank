#pragma once

#include <boost/variant.hpp>

#include <folly/FBVector.h>

#include <jank/runtime/obj/string.hpp>

namespace jank::analyze::expr
{
  /* native/raw expressions start as a string of C++ code which can contain
   * interpolated jank code, but that string is split up into its various pieces
   * for easier codegen. */
  template <typename E>
  struct native_raw
  {
    using chunk_t = boost::variant<runtime::detail::string_type, E>;

    folly::fbvector<chunk_t> chunks;
  };
}
