#pragma once

#include <jank/runtime/obj/string.hpp>

namespace jank::analyze::expr
{
  /* Contains a string of C++ code which can contain interpolated jank code. */
  template <typename E>
  struct native_raw
  {
    runtime::obj::string_ptr code;
    /* TODO: Interpolated expressions. */
  };
}
