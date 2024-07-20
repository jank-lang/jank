#pragma once

namespace jank::runtime
{
  native_persistent_string munge(native_persistent_string const &o);
  object_ptr munge(object_ptr o);
  native_persistent_string demunge(native_persistent_string const &o);
}
