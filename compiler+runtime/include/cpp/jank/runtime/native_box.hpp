#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  template <typename T>
  struct native_box
  {
    using value_type = T;

    native_box() = default;

    native_box(std::nullptr_t)
    {
    }

    native_box(std::remove_const_t<value_type> * const data)
      : data{ data }
    {
    }

    native_box(value_type const * const data)
      : data{ const_cast<value_type *>(data) }
    {
    }

    template <typename C>
    requires std::is_convertible_v<C *, T *>
    native_box(native_box<C> const data)
      : data{ data.data }
    {
    }

    value_type *operator->() const
    {
      assert(data);
      return data;
    }

    native_bool operator!() const
    {
      return !data;
    }

    value_type &operator*() const
    {
      assert(data);
      return *data;
    }

    native_bool operator==(std::nullptr_t) const
    {
      return data == nullptr;
    }

    native_bool operator==(native_box const &rhs) const
    {
      return data == rhs.data;
    }

    native_bool operator!=(std::nullptr_t) const
    {
      return data != nullptr;
    }

    native_bool operator!=(native_box const &rhs) const
    {
      return data != rhs.data;
    }

    native_bool operator<(native_box const &rhs) const
    {
      return data < rhs.data;
    }

    operator native_box<value_type const>() const
    {
      return data;
    }

    operator value_type *() const
    {
      return data;
    }

    operator object *() const
    {
      return &data->base;
    }

    explicit operator native_bool() const
    {
      return data;
    }

    value_type *data{};
  };

  template <>
  struct native_box<object>
  {
    using value_type = object;

    native_box() = default;

    native_box(std::nullptr_t)
    {
    }

    native_box(value_type * const data)
      : data{ data }
    {
    }

    native_box(value_type const * const data)
      : data{ const_cast<value_type *>(data) }
    {
    }

    template <typename T>
    requires behavior::object_like<T>
    native_box(T * const typed_data)
      : data{ &typed_data->base }
    {
    }

    template <typename T>
    requires behavior::object_like<T>
    native_box(T const * const typed_data)
      : data{ typed_data ? const_cast<object *>(&typed_data->base) : nullptr }
    {
    }

    template <typename T>
    requires behavior::object_like<T>
    native_box(native_box<T> const typed_data)
      : data{ typed_data ? &typed_data->base : nullptr }
    {
    }

    value_type *operator->() const
    {
      assert(data);
      return data;
    }

    native_bool operator!() const
    {
      return !data;
    }

    value_type &operator*() const
    {
      assert(data);
      return *data;
    }

    native_bool operator==(std::nullptr_t) const
    {
      return data == nullptr;
    }

    native_bool operator==(native_box const &rhs) const
    {
      return data == rhs.data;
    }

    template <typename T>
    requires behavior::object_like<T>
    native_bool operator==(T const &rhs) const
    {
      return data == &rhs->base;
    }

    template <typename T>
    requires behavior::object_like<T>
    native_bool operator==(native_box<T> const &rhs) const
    {
      return data == &rhs->base;
    }

    native_bool operator!=(std::nullptr_t) const
    {
      return data != nullptr;
    }

    native_bool operator!=(native_box const &rhs) const
    {
      return data != rhs.data;
    }

    template <typename T>
    requires behavior::object_like<T>
    native_bool operator!=(T const &rhs) const
    {
      return data != &rhs->base;
    }

    template <typename T>
    requires behavior::object_like<T>
    native_bool operator!=(native_box<T> const &rhs) const
    {
      return data != &rhs->base;
    }

    native_bool operator<(native_box const &rhs) const
    {
      return data < rhs.data;
    }

    operator native_box<value_type const>() const
    {
      return data;
    }

    operator value_type *() const
    {
      return data;
    }

    explicit operator native_bool() const
    {
      return data;
    }

    value_type *data{};
  };

  template <typename T>
  struct remove_box
  {
    using type = T;
  };

  template <typename T>
  struct remove_box<native_box<T>>
  {
    using type = T;
  };

  template <typename T>
  using remove_box_t = typename remove_box<T>::type;

  template <typename T>
  constexpr native_box<T> make_box(native_box<T> const &o)
  {
    static_assert(sizeof(native_box<T>) == sizeof(T *));
    return o;
  }

  /* TODO: Constexpr these. */
  template <typename T, typename... Args>
  native_box<T> make_box(Args &&...args)
  {
    static_assert(sizeof(native_box<T>) == sizeof(T *));
    native_box<T> ret;
    if constexpr(requires { T::pointer_free; })
    {
      if constexpr(T::pointer_free)
      {
        ret.data = new(PointerFreeGC) T{ std::forward<Args>(args)... };
      }
      else
      {
        ret.data = new(GC) T{ std::forward<Args>(args)... };
      }
    }
    else
    {
      ret.data = new(GC) T{ std::forward<Args>(args)... };
    }

    if(!ret)
    {
      throw std::runtime_error{ "unable to allocate box" };
    }
    return ret;
  }

  template <typename T>
  constexpr native_box<T> make_array_box()
  {
    return nullptr;
  }

  template <typename T, size_t N>
  constexpr native_box<T> make_array_box()
  {
    auto const ret(new(GC) T[N]{});
    if(!ret)
    {
      throw std::runtime_error{ "unable to allocate array box" };
    }
    return ret;
  }

  template <typename T>
  constexpr native_box<T> make_array_box(size_t const length)
  {
    auto const ret(new(GC) T[length]{});
    if(!ret)
    {
      throw std::runtime_error{ "unable to allocate array box" };
    }
    return ret;
  }

  template <typename T, typename... Args>
  native_box<T> make_array_box(Args &&...args)
  {
    /* TODO: pointer_free? */
    auto const ret(new(GC) T[sizeof...(Args)]{ std::forward<Args>(args)... });
    if(!ret)
    {
      throw std::runtime_error{ "unable to allocate array box" };
    }
    return ret;
  }
}
