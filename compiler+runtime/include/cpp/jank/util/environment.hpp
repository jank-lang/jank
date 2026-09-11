#pragma once

#include <jtl/result.hpp>
#include <jtl/immutable_string.hpp>

#include <jank/error.hpp>

namespace jank::util
{
  jtl::immutable_string const &user_home_dir();
  jtl::immutable_string const &user_cache_dir(jtl::immutable_string const &binary_version);
  jtl::immutable_string const &user_config_dir();

  jtl::immutable_string const &binary_version();
  jtl::immutable_string build_dir();

  jtl::immutable_string process_path();
  jtl::immutable_string process_dir();

  jtl::immutable_string resource_dir();
  jtl::result<jtl::immutable_string, error_ref> prelude_hpp_path();

  jtl::immutable_string multi_arch_lib_path();
  void add_system_flags(std::vector<char const *> &args);

  bool is_dynamic_runtime();
}
