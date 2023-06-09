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
  {
    native_box<T> ret;
    if constexpr(T::pointer_free)
    { ret = new (PointerFreeGC) T{ }; }
    else
    { ret = new (GC) T{ }; }

    if(!ret)
    { throw std::runtime_error{ "unable to allocate box" }; }
    return ret;
  }
  template <typename T, typename... Args>
  native_box<T> make_box(Args &&... args)
  {
    native_box<T> ret;
    if constexpr(T::pointer_free)
    { ret = new (PointerFreeGC) T{ std::forward<Args>(args)... }; }
    else
    { ret = new (GC) T{ std::forward<Args>(args)... }; }

    if(!ret)
    { throw std::runtime_error{ "unable to allocate box" }; }
    return ret;
  }
  template <typename T, typename... Args>
  native_box<T> make_array_box(Args &&... args)
  {
    auto const ret(new (GC) T[sizeof...(Args)]{ std::forward<Args>(args)... });
    if(!ret)
    { throw std::runtime_error{ "unable to allocate array box" }; }
    return ret;
  }

  namespace runtime::obj
  {
    struct nil;
    struct boolean;
    struct integer;
    struct real;
    struct string;
    struct list;
  }

  native_box<runtime::obj::nil> make_box(std::nullptr_t const &);
  native_box<runtime::obj::boolean> make_box(native_bool const b);
  native_box<runtime::obj::integer> make_box(int const i);
  native_box<runtime::obj::integer> make_box(native_integer const i);
  native_box<runtime::obj::integer> make_box(size_t const i);
  native_box<runtime::obj::real> make_box(native_real const r);
  native_box<runtime::obj::string> make_box(native_string_view const &s);
}
