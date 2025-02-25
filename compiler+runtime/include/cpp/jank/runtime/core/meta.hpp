#pragma once

#include <jank/read/source.hpp>
#include <jank/runtime/object.hpp>
#include <jank/option.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_hash_map_ptr = native_box<struct persistent_hash_map>;
  }

  object_ptr meta(object_ptr m);
  object_ptr with_meta(object_ptr o, object_ptr m);
  object_ptr reset_meta(object_ptr o, object_ptr m);

  read::source meta_source(option<object_ptr> const &o);
  read::source meta_source(option<object_ptr> const &o, native_persistent_string const &file_path);
  read::source object_source(object_ptr const o);
  read::source object_source(object_ptr const o, native_persistent_string const &file_path);
  obj::persistent_hash_map_ptr
  source_to_meta(read::source_position const &start, read::source_position const &end);
  obj::persistent_hash_map_ptr source_to_meta(object_ptr key,
                                              read::source_position const &start,
                                              read::source_position const &end);
  object_ptr strip_source_from_meta(object_ptr meta);
}
