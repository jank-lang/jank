#include <jank/util/environment.hpp>
#include <jank/error/runtime.hpp>

namespace jank::util
{
  jtl::immutable_string const &binary_version()
  {
    static jtl::immutable_string const res{ "static" };
    return res;
  }

  jtl::immutable_string build_dir()
  {
    throw error::runtime_static_feature_disabled("build_dir");
  }

  bool is_dynamic_runtime()
  {
    return false;
  }

  jtl::result<jtl::immutable_string, error_ref> prelude_hpp_path()
  {
    throw error::runtime_static_feature_disabled("prelude_hpp_path");
  }

  jtl::immutable_string multi_arch_lib_path()
  {
    return "/usr/lib";
  }
}
