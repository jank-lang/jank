#pragma once

#include <jank/read/source.hpp>
#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using persistent_list_ptr = native_box<struct persistent_list>;
  using persistent_list_sequence_ptr = native_box<struct persistent_list_sequence>;
}

namespace jank::read::parse
{
  source reparse_nth(runtime::obj::persistent_list_ptr l, size_t n);
}
