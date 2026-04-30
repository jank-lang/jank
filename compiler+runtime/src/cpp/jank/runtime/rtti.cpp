#include <jank/runtime/rtti.hpp>

namespace jank::runtime
{
  template <>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  obj::small_integer_ref expect_object<obj::small_integer>(object const * const o)
  {
    jank_debug_assert(o);
    jank_debug_assert(o->type == object_type::small_integer);

    return static_cast<obj::small_integer *>(const_cast<object *>(o))->data;
  }
}
