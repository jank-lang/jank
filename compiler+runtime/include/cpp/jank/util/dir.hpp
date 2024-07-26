#pragma once

namespace jank::util
{
  native_persistent_string const &user_home_dir();
  native_persistent_string const &user_cache_dir();
  native_persistent_string const &user_config_dir();
  native_persistent_string const &binary_cache_dir();

  native_persistent_string const &binary_version();
}
