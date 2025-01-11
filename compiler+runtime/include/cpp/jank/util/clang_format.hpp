#pragma once

#include <jank/native_persistent_string.hpp>
#include <jank/result.hpp>

namespace jank::util
{
  struct format_failure
  {
    native_persistent_string reason;
  };

  result<native_persistent_string, format_failure>
  format_cpp_source(native_persistent_string const &);
}
