#pragma once

#include <jank/util/cli.hpp>

namespace jank
{
  struct aot_compiler
  {
    aot_compiler(util::cli::options const &opts);

    void compile(jtl::immutable_string const &module) const;

    native_vector<jtl::immutable_string> include_dirs;
    native_vector<jtl::immutable_string> library_dirs;
    native_vector<jtl::immutable_string> define_macros;
    native_vector<jtl::immutable_string> libs;

    jtl::immutable_string output_filename;
  };
}
