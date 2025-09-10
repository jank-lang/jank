#pragma once

#include <jank/c_api.h>
#include <jank/runtime/object.hpp>

extern "C" jank_object_ref jank_load_clojure_string_native();

namespace clojure::string_native
{
  using namespace jank;
  using namespace jank::runtime;

  object_ref blank(object_ref s);
  object_ref reverse(object_ref s);
  object_ref lower_case(object_ref s);
  object_ref starts_with(object_ref s, object_ref substr);
  object_ref ends_with(object_ref s, object_ref substr);
  object_ref includes(object_ref s, object_ref substr);
  object_ref upper_case(object_ref s);

  i64 index_of(object_ref s, object_ref value, object_ref from_index);
  i64 last_index_of(object_ref s, object_ref value, object_ref from_index);
}
