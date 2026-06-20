#pragma once

#include <jtl/immutable_string.hpp>

#include <jank/runtime/object.hpp>

namespace clojure::data::json_native
{
  using namespace ::jank::runtime;

  struct read_options
  {
    bool eof_error{ true };
    object_ref eof_value{ jank_nil };
    bool bigdec{ false };
    object_ref key_fn{ jank_nil };
    object_ref value_fn{ jank_nil };
    object_ref extra_data_fn{ jank_nil };
  };

  jtl::immutable_string write_str(object_ref const x);
  object_ref read_str(jtl::immutable_string const &string, read_options const &opts);
}
