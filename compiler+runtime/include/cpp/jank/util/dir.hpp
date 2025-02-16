#pragma once

#include <jank/native_persistent_string.hpp>

namespace jank::util
{
  native_persistent_string const &user_home_dir();
  native_persistent_string const &user_cache_dir();
  native_persistent_string const &user_config_dir();
  native_persistent_string const &
  binary_cache_dir(native_integer const optimization_level,
                   native_vector<native_persistent_string> const &includes,
                   native_vector<native_persistent_string> const &defines);

  native_persistent_string const &
  binary_version(native_integer const optimization_level,
                 native_vector<native_persistent_string> const &includes,
                 native_vector<native_persistent_string> const &defines);
}
