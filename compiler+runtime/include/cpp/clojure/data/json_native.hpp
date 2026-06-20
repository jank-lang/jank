#pragma once

#include <jtl/immutable_string.hpp>

#include <jank/runtime/object.hpp>

namespace clojure::data::json_native
{
  using namespace ::jank::runtime;

  struct read_options
  {
    bool eof_error{ true };
    bool bigdec{ false };
    object_ref eof_value{};
    object_ref key_fn{};
    object_ref value_fn{};
  };

  jtl::immutable_string write_str(object_ref const x);
  object_ref read_str(jtl::immutable_string const &string, read_options const &opts);
}
