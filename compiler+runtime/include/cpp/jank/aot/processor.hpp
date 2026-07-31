#pragma once

#include <jank/error.hpp>
#include <jtl/immutable_string.hpp>
#include <jtl/result.hpp>

namespace jank::aot
{
  jtl::result<std::vector<char const *>, error_ref> build_compiler_args();
  std::vector<char const *> build_linker_args();

  struct processor
  {
    jtl::result<void, error_ref> build_executable(jtl::immutable_string const &module) const;
    jtl::result<void, error_ref> compile_object(jtl::immutable_string const &module_name,
                                                jtl::immutable_string const &cpp_source) const;
  };
}
