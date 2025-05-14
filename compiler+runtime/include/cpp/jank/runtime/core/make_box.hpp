#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/native_persistent_list.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/ratio.hpp>
#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/character.hpp>

namespace jank::runtime
{
  /* TODO: Constexpr these. */
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(std::nullptr_t const &)
  {
    return jank_nil;
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(bool const b)
  {
    return b ? jank_true : jank_false;
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(int const i)
  {
    return make_box<obj::integer>(static_cast<i64>(i));
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(i64 const i)
  {
    return make_box<obj::integer>(i);
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(native_big_integer const &i)
  {
    return make_box<obj::big_integer>(i);
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(char const i)
  {
    return make_box<obj::character>(i);
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(usize const i)
  {
    return make_box<obj::integer>(static_cast<i64>(i));
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(f64 const r)
  {
    return make_box<obj::real>(r);
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(obj::ratio_data const &r)
  {
    return make_box<obj::ratio>(r);
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(native_persistent_string_view const &s)
  {
    return make_box<obj::persistent_string>(s);
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline obj::persistent_string_ref make_box(char const * const s)
  {
    jank_assert(s != nullptr);
    return make_box<obj::persistent_string>(s);
  }

  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(detail::native_persistent_list const &l)
  {
    return make_box<obj::persistent_list>(l);
  }

  template <typename T>
  requires std::is_floating_point_v<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(T const d)
  {
    return make_box<obj::real>(d);
  }

  template <typename T>
  requires std::is_integral_v<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(T const d)
  {
    return make_box<obj::integer>(d);
  }

  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(T * const d)
  {
    return d;
  }

  template <typename T>
  requires behavior::object_like<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  inline auto make_box(T const * const d)
  {
    return d;
  }
}
