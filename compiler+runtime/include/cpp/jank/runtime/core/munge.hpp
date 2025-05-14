#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  jtl::immutable_string munge(jtl::immutable_string const &o);
  jtl::immutable_string munge_extra(jtl::immutable_string const &o,
                                    jtl::immutable_string const &search,
                                    char const * const replace);
  object_ref munge(object_ref o);
  jtl::immutable_string demunge(jtl::immutable_string const &o);
}
