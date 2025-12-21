#pragma once

#include <jank/runtime/object.hpp>

namespace clojure::string_native
{
  using namespace jank;
  using namespace jank::runtime;

  object_ref blank(object_ref const s);
  object_ref reverse(object_ref const s);
  object_ref lower_case(object_ref const s);
  object_ref starts_with(object_ref const s, object_ref const substr);
  object_ref ends_with(object_ref const s, object_ref const substr);
  object_ref includes(object_ref const s, object_ref const substr);
  object_ref upper_case(object_ref const s);
  object_ref
  replace_first(object_ref const s, object_ref const match, object_ref const replacement);

  i64 index_of(object_ref const s, object_ref const value, object_ref const from_index);
  i64 last_index_of(object_ref const s, object_ref const value, object_ref const from_index);

  object_ref triml(object_ref const s);
  object_ref trimr(object_ref const s);
  object_ref trim(object_ref const s);
  object_ref trim_newline(object_ref const s);

  object_ref split(object_ref const s, object_ref const re);
  object_ref split(object_ref const s, object_ref const re, object_ref const limit);
}
