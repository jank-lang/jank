#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/native_persistent_list.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using nil = static_object<object_type::nil>;
    using boolean = static_object<object_type::boolean>;
    using integer = static_object<object_type::integer>;
    using real = static_object<object_type::real>;
    using persistent_string = static_object<object_type::persistent_string>;
    using persistent_list = static_object<object_type::persistent_list>;
    using symbol = static_object<object_type::symbol>;
    using character = static_object<object_type::character>;
  }

  /* TODO: Constexpr these. */
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(std::nullptr_t const &)
  {
    return runtime::obj::nil::nil_const();
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(native_bool const b)
  {
    return b ? runtime::obj::boolean::true_const() : runtime::obj::boolean::false_const();
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(int const i)
  {
    return make_box<runtime::obj::integer>(static_cast<native_integer>(i));
  }
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(native_integer const i)
  {
    return make_box<runtime::obj::integer>(i);
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(char const i)
  {
    return make_box<runtime::obj::character>(i);
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(size_t const i)
  {
    return make_box<runtime::obj::integer>(static_cast<native_integer>(i));
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(native_real const r)
  {
    return make_box<runtime::obj::real>(r);
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(native_persistent_string_view const &s)
  {
    return make_box<runtime::obj::persistent_string>(s);
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline runtime::object_ptr make_box(char const * const s)
  {
    if(!s) [[unlikely]]
    {
      return runtime::obj::nil::nil_const();
    }
    return make_box<runtime::obj::persistent_string>(s);
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(runtime::detail::native_persistent_list const &l)
  {
    return make_box<runtime::obj::persistent_list>(l);
  }

  template <typename T>
  requires std::is_floating_point_v<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(T const d)
  {
    return make_box<runtime::obj::real>(d);
  }

  template <typename T>
  requires std::is_integral_v<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(T const d)
  {
    return make_box<runtime::obj::integer>(d);
  }

  template <typename T>
  requires runtime::behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(T * const d)
  {
    return d;
  }

  template <typename T>
  requires runtime::behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(T const * const d)
  {
    return d;
  }

  template <typename T>
  requires runtime::behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(native_box<T> const &d)
  {
    return d;
  }
}
