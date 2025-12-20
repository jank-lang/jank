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

    weak_oref()
      : data{ std::bit_cast<object *>(jank_const_nil()) }
    {
    }

    weak_oref(weak_oref const &rhs) = default;
    weak_oref(weak_oref &&) noexcept = default;

    weak_oref(oref<O> const &rhs)
      : data{ rhs.data }
    {
    }

    weak_oref(nullptr_t) noexcept = delete;

    weak_oref(value_type * const data)
      : data{ data }
    {
      jank_assert_throw(data);
    }

    weak_oref(value_type const * const data)
      : data{ const_cast<value_type *>(data) }
    {
      jank_assert_throw(data);
    }

    template <typename T>
    requires behavior::object_like<T>
    weak_oref(T * const typed_data)
      : data{ &typed_data->base }
    {
      jank_assert_throw(this->data);
    }

    template <typename T>
    requires behavior::object_like<T>
    weak_oref(T const * const typed_data)
      : data{ const_cast<object *>(&typed_data->base) }
    {
      jank_assert_throw(this->data);
    }

    template <typename T>
    requires behavior::object_like<T>
    weak_oref(weak_oref<T> const typed_data) noexcept
      : data{ typed_data.erase().data }
    {
    }

    template <typename T>
    requires behavior::object_like<T>
    weak_oref(oref<T> const &typed_data) noexcept
      : data{ typed_data.erase().data }
    {
    }

    value_type *operator->() const
    {
      jank_assert_throw(data);
      return data;
    }

    value_type &operator*() const
    {
      jank_assert_throw(data);
      return *data;
    }

    weak_oref &operator=(weak_oref const &rhs) noexcept = default;
    weak_oref &operator=(weak_oref &&rhs) noexcept = default;

    template <typename T>
    requires behavior::object_like<T>
    weak_oref &operator=(weak_oref<T> const &rhs) noexcept
    {
      data = &rhs->base;
      return *this;
    }

    bool operator==(weak_oref const &rhs) const noexcept
    {
      return data == rhs.data;
    }

    template <typename T>
    requires behavior::object_like<T>
    bool operator==(weak_oref<T> const &rhs) const noexcept
    {
      return data == rhs.erase().data;
    }

    bool operator!=(weak_oref const &rhs) const noexcept
    {
      return data != rhs.data;
    }

    template <typename T>
    requires behavior::object_like<T>
    bool operator!=(weak_oref<T> const &rhs) const noexcept
    {
      return data != rhs.erase().data;
    }

    weak_oref &operator=(jtl::nullptr_t) noexcept = delete;
    bool operator==(jtl::nullptr_t) noexcept = delete;
    bool operator!=(jtl::nullptr_t) noexcept = delete;

    operator oref<O>() const noexcept
    {
      return data;
    }

    oref<O> strong() const noexcept
    {
      return data;
    }

    value_type *get() const noexcept
    {
      return data;
    }

    weak_oref<O> erase() const noexcept
    {
      return data;
    }

    bool is_some() const noexcept
    {
      return data->type != object_type::nil;
    }

    bool is_nil() const noexcept
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

    weak_oref()
      : data{ std::bit_cast<void *>(jank_const_nil()) }
    {
    }

    weak_oref(nullptr_t) = delete;

    weak_oref(jtl::remove_const_t<T> * const data)
      : data{ data }
    {
      jank_assert_throw(this->data);
    }

    weak_oref(T const * const data)
      : data{ const_cast<T *>(data) }
    {
      jank_assert_throw(this->data);
    }

    template <typename C>
    requires jtl::is_convertible<C *, T *>
    weak_oref(weak_oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires jtl::is_convertible<C *, T *>
    weak_oref(oref<C> const &data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    weak_oref(weak_oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    weak_oref(oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    T *operator->() const
    {
      /* TODO: Add type name. */
      //jank_assert_fmt_throw(*this, "Null reference on weak_oref<{}>", jtl::type_name<T>());
      jank_assert_throw(is_some());
      return reinterpret_cast<T *>(data);
    }

    T &operator*() const
    {
      //jank_assert_fmt_throw(*this, "Null reference on weak_oref<{}>", jtl::type_name<T>());
      jank_assert_throw(is_some());
      return *reinterpret_cast<T *>(data);
    }

    bool operator==(weak_oref<object> const &rhs) const
    {
      return erase().data == rhs.erase().data;
    }

    bool operator!=(weak_oref<object> const &rhs) const
    {
      return erase().data != rhs.erase().data;
    }

    template <typename C>
    requires behavior::object_like<C>
    bool operator==(weak_oref<C> const &rhs) const
    {
      return data == rhs.data;
    }

    template <typename C>
    requires behavior::object_like<C>
    bool operator!=(weak_oref<C> const &rhs) const
    {
      return data != rhs.data;
    }

    weak_oref &operator=(std::remove_cv_t<std::decay_t<T>> * const rhs)
    {
      data = rhs;
      jank_assert_throw(data);
      return *this;
    }

    weak_oref &operator=(std::remove_cv_t<std::decay_t<T>> const * const rhs)
    {
      data = const_cast<T *>(rhs);
      jank_assert_throw(data);
      return *this;
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    weak_oref &operator=(weak_oref<C> const &) noexcept
    {
      data = std::bit_cast<void *>(jank_const_nil());
      return *this;
    }

    weak_oref &operator=(jtl::nullptr_t) noexcept = delete;
    bool operator==(jtl::nullptr_t) noexcept = delete;
    bool operator!=(jtl::nullptr_t) noexcept = delete;

    operator oref<T>() const noexcept
    {
      if(is_nil())
      {
        return {};
      }
      return reinterpret_cast<T *>(data);
    }

    operator oref<object>() const noexcept
    {
      return erase();
    }

    oref<T> strong() const noexcept
    {
      return static_cast<oref<T>>(*this);
    }

    object *get() const noexcept
    {
      return erase();
    }

    weak_oref<object> erase() const noexcept
    {
      if(is_nil())
      {
        return {};
      }
      return &reinterpret_cast<T *>(data)->base;
    }

    bool is_some() const noexcept
    {
      return data != std::bit_cast<void *>(jank_const_nil());
    }

    bool is_nil() const noexcept
    {
      return data == std::bit_cast<void *>(jank_const_nil());
    }

    void *data{};
  };

  template <>
  struct weak_oref<obj::nil>
  {
    using value_type = obj::nil;

    weak_oref()
      : data{ std::bit_cast<value_type *>(jank_const_nil()) }
    {
    }

    weak_oref(nullptr_t) = delete;

    weak_oref(value_type * const data)
      : data{ data }
    {
      jank_assert_throw(this->data);
    }

    weak_oref(value_type const * const data)
      : data{ const_cast<value_type *>(data) }
    {
      jank_assert_throw(this->data);
    }

    template <typename C>
    requires jtl::is_convertible<C *, value_type *>
    weak_oref(oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    weak_oref(weak_oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    weak_oref(oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    value_type *operator->() const
    {
      return data;
    }

    value_type &operator*() const
    {
      return *data;
    }

    bool operator==(weak_oref<object> const &rhs) const
    {
      return rhs.is_nil();
    }

    bool operator!=(weak_oref<object> const &rhs) const
    {
      return rhs.is_some();
    }

    template <typename C>
    requires behavior::object_like<C>
    bool operator==(weak_oref<C> const &rhs) const
    {
      return rhs.is_nil();
    }

    template <typename C>
    requires behavior::object_like<C>
    bool operator!=(weak_oref<C> const &rhs) const
    {
      return rhs.is_some();
    }

    weak_oref &operator=(jtl::nullptr_t) noexcept = delete;
    bool operator==(jtl::nullptr_t) noexcept = delete;
    bool operator!=(jtl::nullptr_t) noexcept = delete;

    template <typename T>
    requires behavior::object_like<T>
    operator oref<T>() const noexcept
    {
      return {};
    }

    operator oref<object>() const noexcept
    {
      return {};
    }

    oref<object> strong() const noexcept
    {
      return {};
    }

    object *get() const noexcept
    {
      return std::bit_cast<object *>(data);
    }

    weak_oref<object> erase() const noexcept
    {
      return std::bit_cast<object *>(data);
    }

    bool is_some() const noexcept
    {
      return false;
    }

    bool is_nil() const noexcept
    {
      return true;
    }

    value_type *data{};
  };

  template <typename T>
  weak_oref<T> make_box(weak_oref<T> const &o)
  {
    static_assert(sizeof(weak_oref<T>) == sizeof(T *));
    return o;
  }
}
