#pragma once

#include <jtl/immutable_string.hpp>
#include <jtl/result.hpp>

namespace jank::util
{
  struct unescape_error
  {
    jtl::immutable_string message;
  };

  /* These provide normal escaping/unescaping, with no quoting. */
  jtl::result<jtl::immutable_string, unescape_error> unescape(jtl::immutable_string const &input);
  jtl::immutable_string escape(jtl::immutable_string const &input);
}
