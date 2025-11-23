#pragma once

#include <jtl/immutable_string.hpp>
#include <jtl/option.hpp>
#include <jtl/result.hpp>

#include <jank/error.hpp>

namespace jank::util
{
  jtl::option<jtl::immutable_string> find_clang();
  jtl::option<jtl::immutable_string> find_clang_resource_dir();
  jtl::result<void, error_ref> invoke_clang(std::vector<char const *> args);

  jtl::option<jtl::immutable_string> find_pch(jtl::immutable_string const &binary_version);
  jtl::result<jtl::immutable_string, error_ref>
  build_pch(std::vector<char const *> args, native_vector<jtl::immutable_string> pch_includes, jtl::immutable_string const &binary_version);

  jtl::immutable_string default_target_triple();
}
