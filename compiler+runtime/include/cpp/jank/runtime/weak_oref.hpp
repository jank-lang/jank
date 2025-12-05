#pragma once

#include <jank/runtime/oref.hpp>

namespace jank::runtime
{
  template <typename T>
  struct weak_oref;

  /* This variation of ref is for type-erased objects.
   *
   * It cannot be null, but it can be nil. */
  template <typename O>
  requires(jtl::is_same<std::decay_t<O>, object>)
  struct weak_oref<O>
  {
    using value_type = object;

    constexpr weak_oref()
      : data{ std::bit_cast<object *>(detail::jank_nil_ptr) }
    {
    }

    constexpr weak_oref(weak_oref const &rhs) = default;
    constexpr weak_oref(weak_oref &&) noexcept = default;

    constexpr weak_oref(oref<O> const &rhs)
      : data{ rhs.data }
    {
    }

    constexpr weak_oref(nullptr_t) noexcept = delete;

    constexpr weak_oref(value_type * const data)
      : data{ data }
    {
      jank_assert_throw(data);
    }

    constexpr weak_oref(value_type const * const data)
      : data{ const_cast<value_type *>(data) }
    {
      jank_assert_throw(data);
    }

    template <typename T>
    requires behavior::object_like<T>
    constexpr weak_oref(T * const typed_data)
      : data{ &typed_data->base }
    {
      jank_assert_throw(this->data);
    }

    template <typename T>
    requires behavior::object_like<T>
    constexpr weak_oref(T const * const typed_data)
      : data{ const_cast<object *>(&typed_data->base) }
    {
      jank_assert_throw(this->data);
    }

    template <typename T>
    requires behavior::object_like<T>
    constexpr weak_oref(weak_oref<T> const typed_data) noexcept
      : data{ typed_data.erase() }
    {
    }

    template <typename T>
    requires behavior::object_like<T>
    constexpr weak_oref(oref<T> const typed_data) noexcept
      : data{ typed_data.erase() }
    {
    }

    constexpr value_type *operator->() const
    {
      jank_assert_throw(data);
      return data;
    }

    constexpr value_type &operator*() const
    {
      jank_assert_throw(data);
      return *data;
    }

    constexpr weak_oref &operator=(weak_oref const &rhs) noexcept = default;
    constexpr weak_oref &operator=(weak_oref &&rhs) noexcept = default;

    template <typename T>
    requires behavior::object_like<T>
    constexpr weak_oref &operator=(weak_oref<T> const &rhs) noexcept
    {
      data = &rhs->base;
      return *this;
    }

    constexpr bool operator==(weak_oref const &rhs) const noexcept
    {
      return data == rhs.data;
    }

    template <typename T>
    requires behavior::object_like<T>
    constexpr bool operator==(weak_oref<T> const &rhs) const noexcept
    {
      return data == rhs.erase();
    }

    constexpr bool operator!=(weak_oref const &rhs) const noexcept
    {
      return data != rhs.data;
    }

    template <typename T>
    requires behavior::object_like<T>
    constexpr bool operator!=(weak_oref<T> const &rhs) const noexcept
    {
      return data != rhs.erase();
    }

    constexpr weak_oref &operator=(jtl::nullptr_t) noexcept = delete;
    constexpr bool operator==(jtl::nullptr_t) noexcept = delete;
    constexpr bool operator!=(jtl::nullptr_t) noexcept = delete;

    constexpr operator oref<O>() const noexcept
    {
      return { data };
    }

    constexpr value_type *get() const noexcept
    {
      return data;
    }

    constexpr value_type *erase() const noexcept
    {
      return data;
    }

    constexpr bool is_some() const noexcept
    {
      return data->type != object_type::nil;
    }

    constexpr bool is_nil() const noexcept
    {
      return data->type == object_type::nil;
    }

    value_type *data{};
  };

  /* This specialization of weak_oref is for fully-typed objects like nil,
   * persistent_list, persistent_array_map, etc.
   *
   * It cannot be null, but it can be nil. */
  template <typename T>
  struct weak_oref
  {
    using value_type = T;

    constexpr weak_oref()
      : data{ std::bit_cast<void *>(detail::jank_nil_ptr) }
    {
    }

    constexpr weak_oref(nullptr_t) = delete;

    constexpr weak_oref(jtl::remove_const_t<T> * const data)
      : data{ data }
    {
      jank_assert_throw(this->data);
    }

    constexpr weak_oref(T const * const data)
      : data{ const_cast<T *>(data) }
    {
      jank_assert_throw(this->data);
    }

    template <typename C>
    requires jtl::is_convertible<C *, T *>
    constexpr weak_oref(weak_oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires jtl::is_convertible<C *, T *>
    constexpr weak_oref(oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    constexpr weak_oref(weak_oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    constexpr weak_oref(oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    constexpr T *operator->() const
    {
      /* TODO: Add type name. */
      //jank_assert_fmt_throw(*this, "Null reference on weak_oref<{}>", jtl::type_name<T>());
      jank_assert_throw(is_some());
      return reinterpret_cast<T *>(data);
    }

    constexpr T &operator*() const
    {
      //jank_assert_fmt_throw(*this, "Null reference on weak_oref<{}>", jtl::type_name<T>());
      jank_assert_throw(is_some());
      return *reinterpret_cast<T *>(data);
    }

    constexpr bool operator==(weak_oref<object> const &rhs) const
    {
      return erase() == rhs.erase();
    }

    constexpr bool operator!=(weak_oref<object> const &rhs) const
    {
      return erase() != rhs.erase();
    }

    template <typename C>
    requires behavior::object_like<C>
    constexpr bool operator==(weak_oref<C> const &rhs) const
    {
      return data == rhs.data;
    }

    template <typename C>
    requires behavior::object_like<C>
    constexpr bool operator!=(weak_oref<C> const &rhs) const
    {
      return data != rhs.data;
    }

    constexpr weak_oref &operator=(std::remove_cv_t<std::decay_t<T>> * const rhs)
    {
      data = rhs;
      jank_assert_throw(data);
      return *this;
    }

    constexpr weak_oref &operator=(std::remove_cv_t<std::decay_t<T>> const * const rhs)
    {
      data = const_cast<T *>(rhs);
      jank_assert_throw(data);
      return *this;
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    constexpr weak_oref &operator=(weak_oref<C> const &) noexcept
    {
      data = std::bit_cast<void *>(detail::jank_nil_ptr);
      return *this;
    }

    constexpr weak_oref &operator=(jtl::nullptr_t) noexcept = delete;
    constexpr bool operator==(jtl::nullptr_t) noexcept = delete;
    constexpr bool operator!=(jtl::nullptr_t) noexcept = delete;

    constexpr operator oref<T>() const noexcept
    {
      if(is_nil())
      {
        return {};
      }
      return reinterpret_cast<T *>(data);
    }

    constexpr operator oref<object>() const noexcept
    {
      return erase();
    }

    constexpr object *get() const noexcept
    {
      return erase();
    }

    constexpr object *erase() const noexcept
    {
      if(is_nil())
      {
        return std::bit_cast<object *>(detail::jank_nil_ptr);
      }
      return &reinterpret_cast<T *>(data)->base;
    }

    constexpr bool is_some() const noexcept
    {
      return data != std::bit_cast<void *>(detail::jank_nil_ptr);
    }

    constexpr bool is_nil() const noexcept
    {
      return data == std::bit_cast<void *>(detail::jank_nil_ptr);
    }

    void *data{};
  };

  template <>
  struct weak_oref<obj::nil>
  {
    using value_type = obj::nil;

    constexpr weak_oref()
      : data{ std::bit_cast<value_type *>(detail::jank_nil_ptr) }
    {
    }

    constexpr weak_oref(nullptr_t) = delete;

    constexpr weak_oref(value_type * const data)
      : data{ data }
    {
      jank_assert_throw(this->data);
    }

    constexpr weak_oref(value_type const * const data)
      : data{ const_cast<value_type *>(data) }
    {
      jank_assert_throw(this->data);
    }

    template <typename C>
    requires jtl::is_convertible<C *, value_type *>
    constexpr weak_oref(weak_oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires jtl::is_convertible<C *, value_type *>
    constexpr weak_oref(oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    constexpr weak_oref(weak_oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    constexpr weak_oref(oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    constexpr value_type *operator->() const
    {
      return data;
    }

    constexpr value_type &operator*() const
    {
      return *data;
    }

    constexpr bool operator==(weak_oref<object> const &rhs) const
    {
      return rhs.is_nil();
    }

    constexpr bool operator!=(weak_oref<object> const &rhs) const
    {
      return rhs.is_some();
    }

    template <typename C>
    requires behavior::object_like<C>
    constexpr bool operator==(weak_oref<C> const &rhs) const
    {
      return rhs.is_nil();
    }

    template <typename C>
    requires behavior::object_like<C>
    constexpr bool operator!=(weak_oref<C> const &rhs) const
    {
      return rhs.is_some();
    }

    constexpr weak_oref &operator=(jtl::nullptr_t) noexcept = delete;
    constexpr bool operator==(jtl::nullptr_t) noexcept = delete;
    constexpr bool operator!=(jtl::nullptr_t) noexcept = delete;

    template <typename T>
    requires behavior::object_like<T>
    constexpr operator oref<T>() const noexcept
    {
      return {};
    }

    constexpr operator oref<object>() const noexcept
    {
      return {};
    }

    constexpr object *get() const noexcept
    {
      return erase();
    }

    constexpr object *erase() const noexcept
    {
      return std::bit_cast<object *>(data);
    }

    constexpr bool is_some() const noexcept
    {
      return false;
    }

    constexpr bool is_nil() const noexcept
    {
      return true;
    }

    value_type *data{};
  };

  template <typename T>
  constexpr weak_oref<T> make_box(weak_oref<T> const &o)
  {
    static_assert(sizeof(weak_oref<T>) == sizeof(T *));
    return o;
  }
}
