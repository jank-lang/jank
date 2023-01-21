#pragma once

namespace jank
{
  /* TODO: Custom ptr in debug which checks for nullptr usage. */
  template <typename T>
  using native_box = T*;

  template <typename T>
  native_box<T> make_box(native_box<T> const &o)
  { return o; }
  template <typename T>
  native_box<T> make_box()
  { return new (GC) T{}; }
  template <typename T, typename... Args>
  native_box<T> make_box(Args &&... args)
  { return new (GC) T{ std::forward<Args>(args)... }; }
}
