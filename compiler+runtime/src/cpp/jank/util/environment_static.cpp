#include <jank/util/environment.hpp>

namespace jank::util
{
  jtl::immutable_string const &binary_version()
  {
    static jtl::immutable_string const res{ "static" };
    return res;
  }
}
