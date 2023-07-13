#pragma once

#include <cstddef>
#include <utility>

namespace jank::obj_model::variant
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

  template <object_type T>
  struct typed_object;

  using object_ptr = struct object*;
}

#include <jank/obj-model/variant/map.hpp>
#include <jank/obj-model/variant/keyword.hpp>

namespace jank::obj_model::variant
{
  using static_nil = typed_object<object_type::nil>;
  template <>
  struct typed_object<object_type::nil>
  {
    typed_object() = default;

    static auto create()
    { return new (PointerFreeGC) static_nil{ }; }
  };

  using variant = boost::variant<typed_object<object_type::nil>, typed_object<object_type::map>, typed_object<object_type::keyword>>;
  struct object : gc
  {
    variant data{};
  };

  //template <typename T>
  //object_ptr erase_type(T * const t)
  //{ return &t->base; }
  inline object_ptr erase_type(object * const t)
  { return t; }


  template <object_type T, typename... Args>
  auto make_object(Args && ...args)
  { return new (GC) object{ {}, typed_object<T>{ std::forward<Args>(args)... } }; }
}
