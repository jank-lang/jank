#pragma once

#include <functional>
#include <vector>

#include <jtl/immutable_string.hpp>
#include <jtl/option.hpp>
#include <jtl/result.hpp>

#include <jank/error.hpp>

namespace llvm
{
  class Module;
}

namespace jank::util
{
  jtl::option<jtl::immutable_string> find_clang();
  jtl::option<jtl::immutable_string> find_clang_resource_dir();
  jtl::result<void, error_ref> invoke_clang(std::vector<char const *> args);

  /* Compiles a single translation unit to an object file, like `invoke_clang`, but hands the
   * LLVM module to `transform` in memory after codegen and before optimization and object
   * emission. `args` must request exactly one object-producing compile job. */
  jtl::result<void, error_ref>
  invoke_clang_with_module_transform(std::vector<char const *> args,
                                     std::function<void(llvm::Module &)> const &transform);

  jtl::option<jtl::immutable_string> find_pch(jtl::immutable_string const &binary_version);
  jtl::result<jtl::immutable_string, error_ref>
  build_pch(std::vector<char const *> args, jtl::immutable_string const &binary_version);

  jtl::immutable_string default_target_triple();
}
