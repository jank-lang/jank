#pragma once

#include <magic_enum.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  /* Most of the system is polymorphic using type-erased objects. Given any object, an erase call
   * will get you what you need. If you don't need to type-erase, though, don't! */
  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr object_ptr erase(native_box<T> const o)
  {
    return &o->base;
  }

  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr object_ptr erase(native_box<T const> const o)
  {
    return const_cast<object_ptr>(&o->base);
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr object_ptr erase(object_ptr const o)
  {
    return o;
  }

  template <typename T>
  requires behavior::object_like<std::decay_t<std::remove_pointer_t<T>>>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr object_ptr erase(T const o)
  {
    return const_cast<object *>(&o->base);
  }

  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr native_box<T> isa(object const * const o)
  {
    assert(o);
    return o->type == T::obj_type;
  }

  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr native_box<T> dyn_cast(object const * const o)
  {
    assert(o);
    if(o->type != T::obj_type)
    {
      return nullptr;
    }
    return reinterpret_cast<T *>(reinterpret_cast<char *>(const_cast<object *>(o))
                                 - offsetof(T, base));
  }

  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr native_box<T> try_object(object const * const o)
  {
    assert(o);
    if(o->type != T::obj_type)
    {
      throw std::runtime_error{ fmt::format("invalid object type (expected {}, found {})",
                                            magic_enum::enum_name(T::obj_type),
                                            magic_enum::enum_name(o->type)) };
    }
    return reinterpret_cast<T *>(reinterpret_cast<char *>(const_cast<object *>(o))
                                 - offsetof(T, base));
  }

  /* This is dangerous. You probably don't want it. Just use `try_object` or `visit_object`.
   * However, if you're absolutely certain you know the type of an erased object, I guess
   * you can use this. */
  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr native_box<T> expect_object(object_ptr const o)
  {
    assert(o);
    assert(o->type == T::obj_type);
    return reinterpret_cast<T *>(reinterpret_cast<char *>(o.data) - offsetof(T, base));
  }

  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr native_box<T> expect_object(object const * const o)
  {
    assert(o);
    assert(o->type == T::obj_type);
    return reinterpret_cast<T *>(reinterpret_cast<char *>(const_cast<object *>(o))
                                 - offsetof(T, base));
  }
}
