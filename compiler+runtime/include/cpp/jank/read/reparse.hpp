#pragma once

#include <jank/read/source.hpp>
#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using persistent_list_ptr = native_box<struct persistent_list>;
  using persistent_list_sequence_ptr = native_box<struct persistent_list_sequence>;
  using persistent_vector_ptr = native_box<struct persistent_vector>;
  using persistent_vector_sequence_ptr = native_box<struct persistent_vector_sequence>;
}

/* Reparsing allows us to retrieve source information for parsed objects which
 * can't hold their own source information as meta. For example, keywords and numbers
 * don't have meta.
 *
 * To remedy this, when we have some code like `(def :foo 1)` and we want to point at
 * `:foo`, we take the full list, which we know has meta, and we go back to the source
 * to parse the nth form inside that list. That gives us the source for `:foo`. */
namespace jank::read::parse
{
  source reparse_nth(runtime::obj::persistent_list_ptr o, size_t n);
  source reparse_nth(runtime::obj::persistent_vector_ptr o, size_t n);
  source reparse_nth(runtime::object_ptr o, size_t n);
}
