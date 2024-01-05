#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/native_persistent_list.hpp>

namespace jank::runtime::detail
{
  native_persistent_string to_string(object_ptr const o);
  native_integer to_hash(object_ptr const o);
  native_real to_real(object_ptr const o);
  void to_string(object_ptr const o, fmt::memory_buffer &buff);
  native_bool equal(object_ptr const lhs, object_ptr const rhs);

  template <typename T>
  struct object_type_to_enum;

  template <object_type O>
  struct object_type_to_enum<static_object<O>>
  { static constexpr object_type value{ O }; };
}

namespace jank
{
  namespace runtime::obj
  {
    using nil = static_object<object_type::nil>;
    using boolean = static_object<object_type::boolean>;
    using integer = static_object<object_type::integer>;
    using real = static_object<object_type::real>;
    using string = static_object<object_type::string>;
    using list = static_object<object_type::list>;
    using symbol = static_object<object_type::symbol>;
  }

  /* TODO: Constexpr these. */

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(std::nullptr_t const &)
  { return runtime::obj::nil::nil_const(); }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(native_bool const b)
  { return b ? runtime::obj::boolean::true_const() : runtime::obj::boolean::false_const(); }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(int const i)
  { return make_box<runtime::obj::integer>(static_cast<native_integer>(i)); }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(native_integer const i)
  { return make_box<runtime::obj::integer>(i); }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(size_t const i)
  { return make_box<runtime::obj::integer>(static_cast<native_integer>(i)); }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(native_real const r)
  { return make_box<runtime::obj::real>(r); }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(native_persistent_string_view const &s)
  { return make_box<runtime::obj::string>(s); }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(char const * const s)
  { return make_box<runtime::obj::string>(s); }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(runtime::detail::native_persistent_list const &l)
  { return make_box<runtime::obj::list>(l); }

  template <typename T>
  requires std::is_floating_point_v<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(T const d)
  { return make_box<runtime::obj::real>(d); }

  template <typename T>
  requires std::is_integral_v<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(T const d)
  { return make_box<runtime::obj::integer>(d); }

  template <typename T>
  requires runtime::behavior::objectable<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(T * const d)
  { return d; }

  template <typename T>
  requires runtime::behavior::objectable<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(T const * const d)
  { return d; }

  template <typename T>
  requires runtime::behavior::objectable<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(native_box<T> const &d)
  { return d; }
}

namespace std
{
  template <>
  struct hash<jank::runtime::object_ptr>
  { size_t operator()(jank::runtime::object_ptr const o) const noexcept; };

  template <>
  struct hash<jank::runtime::object>
  { size_t operator()(jank::runtime::object const &o) const noexcept; };

  template <>
  struct equal_to<jank::runtime::object_ptr>
  { bool operator()(jank::runtime::object_ptr const lhs, jank::runtime::object_ptr const rhs) const noexcept; };
}
