#pragma once

#include <regex>
#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  native_persistent_string munge(native_persistent_string const &o);
  native_persistent_string munge_extra(native_persistent_string const &o,
                                       std::regex const &search,
                                       char const * const replace);
  object_ptr munge(object_ptr o);
  native_persistent_string demunge(native_persistent_string const &o);
}
