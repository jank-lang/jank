#pragma once

#include <jtl/immutable_string.hpp>

#include <jank/runtime/object.hpp>

namespace clojure::data::json_native
{
  jtl::immutable_string write_str(jank::runtime::object_ref const x);
  jank::runtime::object_ref read_str(jtl::immutable_string const &string);
}
