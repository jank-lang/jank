#pragma once

#include <jank/read/source.hpp>
#include <jank/runtime/object.hpp>
#include <jank/option.hpp>

namespace jank::runtime
{
  object_ptr meta(object_ptr m);
  object_ptr with_meta(object_ptr o, object_ptr m);
  object_ptr reset_meta(object_ptr o, object_ptr m);

  read::source meta_source(option<object_ptr> const &o);
  read::source object_source(object_ptr const o);
}
