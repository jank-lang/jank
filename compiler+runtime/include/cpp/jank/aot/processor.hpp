#pragma once

#include <jtl/immutable_string.hpp>
#include <jtl/result.hpp>

namespace jank::util::cli
{
  struct options;
}

namespace jank::aot
{
  struct compiler_err
  {
    compiler_err(int const &return_code, jtl::immutable_string const &err_message)
      : return_code{ return_code }
      , err_message{ err_message }
    {
    }

    int return_code;
    jtl::immutable_string err_message;
  };

  struct processor
  {
    processor(util::cli::options const &opts);

    jtl::result<void, compiler_err> compile(jtl::immutable_string const &module) const;

    native_vector<jtl::immutable_string> include_dirs;
    native_vector<jtl::immutable_string> library_dirs;
    native_vector<jtl::immutable_string> define_macros;
    native_vector<jtl::immutable_string> libs;

    jtl::immutable_string output_filename;
  };

}
