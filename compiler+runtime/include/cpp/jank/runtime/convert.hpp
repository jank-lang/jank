#pragma once

#include <jtl/trait/predicate.hpp>
#include <jtl/memory.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  /* This is a general trait for converting to/from jank's object types.
   * Conversion can be used anywhere in your C++ code, but it's also used by
   * jank's seamless interop for automatically conversions when passing
   * native types as args to jank fns or jank objects are args to native fns.
   *
   * Each specialization of it needs to implement two static functions:
   *
   * 1. to_object
   * 2. from_object
   *
   * The input of `to_object` will always be a `T` and the output can be whichever
   * boxed object type is fitting. As always, prefer to use fully typed objects like
   * `obj::persistent_hash_map_ref` instead of `object_ref`.
   *
   * Conversely, `from_object` will take an `object_ref` and turn it into a `T`. This
   * is the minimum, but if more overloads to `from_object` are provided, which use
   * typed objects, the jank system will use them.
   *
   * Check out `convert/builtin.hpp` for examples. */
  template <typename T>
  struct convert;

  template <typename T>
  concept typed_object_ref = std::same_as<T, oref<typename T::value_type>>;

  template <typename T>
  concept untyped_object_ref = jtl::is_same<T, object_ref>;

  template <typename T>
  concept any_object_ref = (typed_object_ref<T> || untyped_object_ref<T>);

  template <typename T>
  concept convertible_to_object = requires(T const &t) {
    { convert<T>::to_object(t) } -> any_object_ref;
  };

  template <typename T>
  concept convertible_from_object = requires(object_ref const o) {
    { convert<T>::from_object(o) } -> jtl::is_convertible<T>;
  };

  template <typename T>
  concept convertible = (convertible_to_object<T> && convertible_from_object<T>);

  template <typename T>
  concept has_conversion_members = requires(T const &t, object_ref const o) {
    { T::into_object(t) } -> any_object_ref;
    { T::from_object(o) } -> jtl::is_convertible<T>;
  };

  /* Any object can convert to/from itself easily. */
  template <typename T>
  requires any_object_ref<T>
  struct convert<T>
  {
    static constexpr T into_object(T const t)
    {
      return t;
    }

    static constexpr T from_object(T const t)
    {
      return t;
    }
  };

  /* Any type with conversion members can be convertible. */
  template <typename T>
  requires has_conversion_members<T>
  struct convert<T>
  {
    static constexpr auto into_object(T const &t)
    {
      return T::into_object(t);
    }

    static constexpr T from_object(auto const o)
    {
      return T::from_object(o);
    }
  };
}
