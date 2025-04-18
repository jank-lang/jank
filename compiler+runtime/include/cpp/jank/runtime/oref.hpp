#pragma once

#include <type_traits>

#include <jtl/trait/transform.hpp>
#include <jtl/ref.hpp>
#include <jtl/ptr.hpp>
#include <jtl/assert.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  namespace obj
  {
    struct nil;
  }

  namespace detail
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
    extern obj::nil *jank_nil_ptr;
  }

  template <typename T>
  struct oref;

  /* This variation of ref is for type-erased objects.
   *
   * It cannot be null, but it can be nil. */
  template <typename O>
  requires(jtl::is_same<std::decay_t<O>, object>)
  struct oref<O>
  {
    using value_type = object;

    constexpr oref()
      : data{ std::bit_cast<object *>(detail::jank_nil_ptr) }
    {
    }

    constexpr oref(nullptr_t) noexcept = delete;

    constexpr oref(value_type * const data)
      : data{ data }
    {
      jank_assert_throw(data);
    }

    constexpr oref(value_type const * const data)
      : data{ const_cast<value_type *>(data) }
    {
      jank_assert_throw(data);
    }

    template <typename T>
    requires behavior::object_like<T>
    constexpr oref(T * const typed_data)
      : data{ &typed_data->base }
    {
      jank_assert_throw(this->data);
    }

    template <typename T>
    requires behavior::object_like<T>
    constexpr oref(T const * const typed_data)
      : data{ const_cast<object *>(&typed_data->base) }
    {
      jank_assert_throw(this->data);
    }

    template <typename T>
    requires behavior::object_like<T>
    constexpr oref(oref<T> const typed_data) noexcept
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

    constexpr bool operator==(oref const &rhs) const noexcept
    {
      return data == rhs.data;
    }

    template <typename T>
    requires behavior::object_like<T>
    constexpr bool operator==(oref<T> const &rhs) const noexcept
    {
      return data == rhs.erase();
    }

    constexpr bool operator!=(oref const &rhs) const noexcept
    {
      return data != rhs.data;
    }

    template <typename T>
    requires behavior::object_like<T>
    constexpr bool operator!=(oref<T> const &rhs) const noexcept
    {
      return data != rhs.erase();
    }

    constexpr oref &operator=(jtl::nullptr_t) noexcept = delete;
    constexpr bool operator==(jtl::nullptr_t) noexcept = delete;
    constexpr bool operator!=(jtl::nullptr_t) noexcept = delete;

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

  /* This specialization of oref is for fully-typed objects like nil,
   * persistent_list, persistent_array_map, etc.
   *
   * It cannot be null, but it can be nil. */
  template <typename T>
  struct oref
  {
    using value_type = T;

    constexpr oref()
      : data{ std::bit_cast<void *>(detail::jank_nil_ptr) }
    {
    }

    constexpr oref(nullptr_t) = delete;

    constexpr oref(jtl::remove_const_t<T> * const data)
      : data{ data }
    {
      jank_assert_throw(this->data);
    }

    constexpr oref(T const * const data)
      : data{ const_cast<T *>(data) }
    {
      jank_assert_throw(this->data);
    }

    template <typename C>
    requires jtl::is_convertible<C *, T *>
    constexpr oref(oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    constexpr oref(oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    constexpr T *operator->() const
    {
      /* TODO: Add type name. */
      //jank_assert_fmt_throw(*this, "Null reference on oref<{}>", jtl::type_name<T>());
      jank_assert_throw(is_some());
      return reinterpret_cast<T *>(data);
    }

    constexpr T &operator*() const
    {
      //jank_assert_fmt_throw(*this, "Null reference on oref<{}>", jtl::type_name<T>());
      jank_assert_throw(is_some());
      return *reinterpret_cast<T *>(data);
    }

    constexpr bool operator==(oref<object> const &rhs) const
    {
      return !(*this != rhs);
    }

    constexpr bool operator!=(oref<object> const &rhs) const
    {
      return erase() == rhs;
    }

    template <typename C>
    requires behavior::object_like<C>
    constexpr bool operator==(oref<C> const &rhs) const
    {
      return data == rhs.data;
    }

    template <typename C>
    requires behavior::object_like<C>
    constexpr bool operator!=(oref<C> const &rhs) const
    {
      return data != rhs.data;
    }

    constexpr oref &operator=(std::remove_cv_t<std::decay_t<T>> * const rhs)
    {
      data = rhs;
      jank_assert_throw(data);
      return *this;
    }

    constexpr oref &operator=(std::remove_cv_t<std::decay_t<T>> const * const rhs)
    {
      data = const_cast<T *>(rhs);
      jank_assert_throw(data);
      return *this;
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    constexpr oref &operator=(oref<C> const &) noexcept
    {
      data = std::bit_cast<void *>(detail::jank_nil_ptr);
      return *this;
    }

    constexpr oref &operator=(jtl::nullptr_t) noexcept = delete;
    constexpr bool operator==(jtl::nullptr_t) noexcept = delete;
    constexpr bool operator!=(jtl::nullptr_t) noexcept = delete;

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
  struct oref<obj::nil>
  {
    using value_type = obj::nil;

    constexpr oref()
      : data{ std::bit_cast<value_type *>(detail::jank_nil_ptr) }
    {
    }

    constexpr oref(nullptr_t) = delete;

    constexpr oref(value_type * const data)
      : data{ data }
    {
      jank_assert_throw(this->data);
    }

    constexpr oref(value_type const * const data)
      : data{ const_cast<value_type *>(data) }
    {
      jank_assert_throw(this->data);
    }

    template <typename C>
    requires jtl::is_convertible<C *, value_type *>
    constexpr oref(oref<C> const data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    constexpr oref(oref<C> const data) noexcept
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

    constexpr bool operator==(oref<object> const &rhs) const
    {
      return rhs.is_nil();
    }

    constexpr bool operator!=(oref<object> const &rhs) const
    {
      return rhs.is_some();
    }

    template <typename C>
    requires behavior::object_like<C>
    constexpr bool operator==(oref<C> const &rhs) const
    {
      return rhs.is_nil();
    }

    template <typename C>
    requires behavior::object_like<C>
    constexpr bool operator!=(oref<C> const &rhs) const
    {
      return rhs.is_some();
    }

    constexpr oref &operator=(jtl::nullptr_t) noexcept = delete;
    constexpr bool operator==(jtl::nullptr_t) noexcept = delete;
    constexpr bool operator!=(jtl::nullptr_t) noexcept = delete;

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
  constexpr jtl::ref<T> make_box(jtl::ref<T> const &o)
  {
    static_assert(sizeof(jtl::ref<T>) == sizeof(T *));
    return o;
  }

  template <typename T>
  constexpr oref<T> make_box(oref<T> const &o)
  {
    static_assert(sizeof(oref<T>) == sizeof(T *));
    return o;
  }

  /* TODO: Constexpr these. */
  template <typename T, typename... Args>
  jtl::ref<T> make_box(Args &&...args)
  {
    static_assert(sizeof(jtl::ref<T>) == sizeof(T *));
    T *ret{};
    if constexpr(requires { T::pointer_free; })
    {
      if constexpr(T::pointer_free)
      {
        ret = new(PointerFreeGC) T{ std::forward<Args>(args)... };
      }
      else
      {
        ret = new(GC) T{ std::forward<Args>(args)... };
      }
    }
    else
    {
      ret = new(GC) T{ std::forward<Args>(args)... };
    }

    if(!ret)
    {
      /* TODO: Panic. */
      throw std::runtime_error{ "unable to allocate box" };
    }
    return ret;
  }

  template <typename T, typename... Args>
  requires behavior::object_like<T>
  oref<T> make_box(Args &&...args)
  {
    static_assert(sizeof(oref<T>) == sizeof(T *));
    oref<T> ret;
    if constexpr(requires { T::pointer_free; })
    {
      if constexpr(T::pointer_free)
      {
        ret = new(PointerFreeGC) T{ std::forward<Args>(args)... };
      }
      else
      {
        ret = new(GC) T{ std::forward<Args>(args)... };
      }
    }
    else
    {
      ret = new(GC) T{ std::forward<Args>(args)... };
    }

    return ret;
  }

  /* TODO: This no longer makes sense. */
  template <typename T>
  constexpr jtl::ref<T> make_array_box()
  {
    return nullptr;
  }

  template <typename T, usize N>
  constexpr jtl::ref<T> make_array_box()
  {
    auto const ret(new(GC) T[N]{});
    if(!ret)
    {
      throw std::runtime_error{ "unable to allocate array box" };
    }
    return ret;
  }

  template <typename T>
  constexpr jtl::ref<T> make_array_box(usize const length)
  {
    auto const ret(new(GC) T[length]{});
    if(!ret)
    {
      throw std::runtime_error{ "Unable to allocate array box" };
    }
    return ret;
  }

  template <typename T, typename... Args>
  jtl::ref<T> make_array_box(Args &&...args)
  {
    /* TODO: pointer_free? */
    auto const ret(new(GC) T[sizeof...(Args)]{ std::forward<Args>(args)... });
    if(!ret)
    {
      throw std::runtime_error{ "Unable to allocate array box" };
    }
    return ret;
  }

  template <typename D, typename B>
  jtl::ref<D> static_box_cast(jtl::ref<B> const ptr) noexcept
  {
    return static_cast<D *>(ptr.data);
  }

}
