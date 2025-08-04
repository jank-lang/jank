#pragma once

#include <jtl/immutable_string.hpp>

namespace jank::util
{
  jtl::immutable_string const &user_home_dir();
  jtl::immutable_string const &user_cache_dir(jtl::immutable_string const &binary_version);
  jtl::immutable_string const &user_config_dir();
  jtl::immutable_string const &binary_cache_dir(jtl::immutable_string const &binary_version);

  jtl::immutable_string const &binary_version();

  jtl::immutable_string process_path();
  jtl::immutable_string process_dir();
}
