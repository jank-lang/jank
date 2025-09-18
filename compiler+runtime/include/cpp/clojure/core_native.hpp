#pragma once

#include <jank/c_api.h>
#include <jank/runtime/object.hpp>

extern "C" jank_object_ref jank_load_clojure_core_native();

namespace clojure::core_native
{
  using namespace jank::runtime;

  object_ref is_var(object_ref o);
  object_ref var_get(object_ref o);
  object_ref alter_var_root(object_ref o, object_ref fn, object_ref args);

  object_ref ns_unalias(object_ref current_ns, object_ref alias);
  object_ref ns_unmap(object_ref current_ns, object_ref sym);

  object_ref hash_unordered(object_ref coll);
  object_ref jank_version();
}
