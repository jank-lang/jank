#pragma once

#include <jank/error.hpp>
#include <jtl/immutable_string.hpp>
#include <jtl/result.hpp>

namespace jank::aot
{
  struct processor
  {
    jtl::result<void, error_ref> compile(jtl::immutable_string const &module) const;
  };
}
