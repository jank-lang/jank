#pragma once

#include <cstddef>
#include <utility>

namespace jank::obj_model::tagged
{
  enum class object_type
  {
    nil = 1,
    integer,
    real,
    number,
    keyword,
    map
  };

  struct object
  { object_type type{}; };
  using object_ptr = object*;

  template <object_type T>
  struct typed_object;

  template <typename T>
  object_ptr erase_type(T * const t)
  {
    if constexpr(std::is_same_v<T, object>)
    { return t; }
    else
    { return &t->base; }
  }

  using static_nil = typed_object<object_type::nil>;
  template <>
  struct typed_object<object_type::nil> : gc
  {
    typed_object() = default;
    typed_object(object &&base)
      : base{ std::move(base) }
    { }

    static auto create()
    { return new (PointerFreeGC) static_nil{ }; }

    object base{ object_type::nil };
  };

  template <object_type T, typename... Args>
  auto make_object(Args && ...args)
  {
    using TO = typed_object<T>;
    static_assert(offsetof(TO, base) == 0, "object base; needs to be the first member of each typed object");
    return new (GC) typed_object<T>{ { T }, std::forward<Args>(args)... };
  }
}
