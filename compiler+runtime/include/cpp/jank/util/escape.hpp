#pragma once

#include <jank/native_persistent_string.hpp>
#include <jtl/result.hpp>

namespace jank::util
{
  struct unescape_error
  {
    native_persistent_string message;
  };

  /* These provide normal escaping/unescaping, with no quoting. */
  jtl::result<native_persistent_string, unescape_error> unescape(native_persistent_string const &input);
  native_persistent_string escape(native_persistent_string const &input);
}
