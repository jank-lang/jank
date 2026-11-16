#pragma once

#include <filesystem>
#include <jtl/immutable_string.hpp>

namespace jank::aot
{
  struct target_config
  {
    jtl::immutable_string triple;
    std::filesystem::path clang_bin;
    jtl::immutable_string jit_flags;
    jtl::immutable_string aot_flags;
  };

  jtl::result<target_config, error_ref> load_target_config(std::string const &path);
}
