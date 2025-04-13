#pragma once

#include <jtl/option.hpp>

#include <jank/read/source.hpp>
#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_hash_map_ref = jtl::oref<struct persistent_hash_map>;
  }

  object_ptr meta(object_ptr m);
  object_ptr with_meta(object_ptr o, object_ptr m);
  object_ptr with_meta_graceful(object_ptr o, object_ptr m);
  object_ptr reset_meta(object_ptr o, object_ptr m);

  read::source meta_source(jtl::option<object_ptr> const &o);
  read::source object_source(object_ptr const o);
  obj::persistent_hash_map_ref
  source_to_meta(read::source_position const &start, read::source_position const &end);
  obj::persistent_hash_map_ref source_to_meta(object_ptr key,
                                              read::source_position const &start,
                                              read::source_position const &end);
  object_ptr strip_source_from_meta(object_ptr meta);
}
