#pragma once

#include <jtl/option.hpp>

#include <jank/read/source.hpp>
#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_hash_map_ref = oref<struct persistent_hash_map>;
  }

  object_ref meta(object_ref const m);
  object_ref with_meta(object_ref const o, object_ref const m);
  object_ref with_meta_graceful(object_ref const o, object_ref const m);
  object_ref reset_meta(object_ref const o, object_ref const m);

  read::source meta_source(jtl::option<object_ref> const &o);
  read::source object_source(object_ref const o);
  obj::persistent_hash_map_ref source_to_meta(read::source const &source);
  obj::persistent_hash_map_ref
  source_to_meta(read::source_position const &start, read::source_position const &end);
  object_ref strip_source_from_meta(object_ref const meta);
  jtl::option<object_ref> strip_source_from_meta_opt(jtl::option<object_ref> const &meta);
}
