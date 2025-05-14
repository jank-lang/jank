#pragma once

#include <jtl/immutable_string.hpp>

namespace jank::util
{
  jtl::immutable_string const &user_home_dir();
  jtl::immutable_string const &user_cache_dir();
  jtl::immutable_string const &user_config_dir();
  jtl::immutable_string const &
  binary_cache_dir(i64 const optimization_level,
                   native_vector<jtl::immutable_string> const &includes,
                   native_vector<jtl::immutable_string> const &defines);

  jtl::immutable_string const &binary_version(i64 const optimization_level,
                                              native_vector<jtl::immutable_string> const &includes,
                                              native_vector<jtl::immutable_string> const &defines);
}
