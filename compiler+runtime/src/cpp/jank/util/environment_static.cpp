#include <jank/util/environment.hpp>
#include <jank/error/runtime.hpp>

namespace jank::util
{
  jtl::immutable_string const &binary_version()
  {
    static jtl::immutable_string const res{ "static" };
    return res;
  }

  jtl::immutable_string build_version()
  {
    throw error::runtime_static_feature_disabled("eval");
  }
}
