#pragma once

#include <jank/error.hpp>
#include <jtl/immutable_string.hpp>
#include <jtl/result.hpp>

namespace jank::util::cli
{
  struct options;
}

namespace jank::aot
{
  struct processor
  {
    processor(util::cli::options const &opts);

    jtl::result<void, error_ref> compile(jtl::immutable_string const &module) const;

    native_vector<jtl::immutable_string> include_dirs;
    native_vector<jtl::immutable_string> library_dirs;
    native_vector<jtl::immutable_string> define_macros;
    native_vector<jtl::immutable_string> libs;

    jtl::immutable_string output_filename;
  };

}
