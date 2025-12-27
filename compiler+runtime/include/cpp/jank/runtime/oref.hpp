#pragma once

#include <type_traits>

#include <jtl/trait/transform.hpp>
#include <jtl/ref.hpp>
#include <jtl/ptr.hpp>
#include <jtl/assert.hpp>

#include <jank/runtime/object.hpp>

extern "C" void *jank_const_nil();

namespace jank::runtime
{
  namespace obj
  {
    struct nil;
    struct boolean;
  }

  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  extern oref<struct obj::boolean> jank_true;
  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  extern oref<struct obj::boolean> jank_false;

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

    oref() = default;
    oref(oref const &rhs) = default;
    oref(oref &&rhs) noexcept = default;

    oref(nullptr_t) noexcept = delete;

    oref(value_type * const data) noexcept
      : data{ data }
    {
      jank_assert(data);
    }

    oref(value_type const * const data) noexcept
      : data{ const_cast<value_type *>(data) }
    {
      jank_assert(data);
    }

    template <typename T>
    requires behavior::object_like<T>
    oref(T * const typed_data) noexcept
      : data{ &typed_data->base }
    {
      jank_assert(this->data);
    }

    template <typename T>
    requires behavior::object_like<T>
    oref(T const * const typed_data) noexcept
      : data{ const_cast<object *>(&typed_data->base) }
    {
      jank_assert(this->data);
    }

    template <typename T>
    requires behavior::object_like<T>
    oref(oref<T> const &typed_data) noexcept
      : data{ typed_data.erase().data }
    {
    }

    ~oref() = default;

    void reset() noexcept
    {
      data = std::bit_cast<object *>(jank_const_nil());
    }

    void reset(object * const o) noexcept
    {
      data = o;
    }

    void reset(oref<object> const &o) noexcept
    {
      data = o.data;
    }

    value_type *operator->() const noexcept
    {
      jank_assert(data);
      return data;
    }

    value_type &operator*() const noexcept
    {
      jank_assert(data);
      return *data;
    }

    oref &operator=(oref const &rhs) noexcept = default;
    oref &operator=(oref &&rhs) noexcept = default;

    template <typename T>
    requires behavior::object_like<T>
    oref &operator=(oref<T> const &rhs) noexcept
    {
      if(data == &rhs->base)
      {
        return *this;
      }

      data = &rhs->base;
      return *this;
    }

    bool operator==(oref const &rhs) const noexcept
    {
      return data == rhs.data;
    }

    template <typename T>
    requires behavior::object_like<T>
    bool operator==(oref<T> const &rhs) const noexcept
    {
      return data == rhs.erase().data;
    }

    bool operator!=(oref const &rhs) const noexcept
    {
      return data != rhs.data;
    }

    template <typename T>
    requires behavior::object_like<T>
    bool operator!=(oref<T> const &rhs) const noexcept
    {
      return data != rhs.erase().data;
    }

    oref &operator=(jtl::nullptr_t) noexcept = delete;
    bool operator==(jtl::nullptr_t) noexcept = delete;
    bool operator!=(jtl::nullptr_t) noexcept = delete;

    value_type *get() const noexcept
    {
      return data;
    }

    oref<object> const &erase() const noexcept
    {
      return *this;
    }

    bool is_some() const noexcept
    {
      /* NOLINTNEXTLINE(clang-analyzer-core.NullDereference): I cannot see how this can happen. We initialize to non-null and always ensure non-null on mutation. That's the whole point of this type. */
      return data->type != object_type::nil;
    }

    bool is_nil() const noexcept
    {
      return data->type == object_type::nil;
    }

    value_type *data{ std::bit_cast<object *>(jank_const_nil()) };
  };

  /* This specialization of oref is for fully-typed objects like
   * persistent_list, persistent_array_map, etc.
   *
   * It cannot be null, but it can be nil. */
  template <typename T>
  struct oref
  {
    using value_type = T;

    oref() = default;
    oref(oref const &rhs) noexcept = default;
    oref(oref &&rhs) noexcept = default;

    oref(nullptr_t) = delete;

    oref(jtl::remove_const_t<T> * const data) noexcept
      : data{ data }
    {
      jank_assert(this->data);
    }

    oref(T const * const data) noexcept
      : data{ const_cast<T *>(data) }
    {
      jank_assert(this->data);
    }

    template <typename C>
    requires jtl::is_convertible<C *, T *>
    oref(oref<C> const &data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    oref(oref<C> const &data) noexcept
      : data{ data.data }
    {
    }

    ~oref() = default;

    void reset() noexcept
    {
      data = std::bit_cast<object *>(jank_const_nil());
    }

    void reset(object * const o) noexcept
    {
      data = o;
    }

    void reset(oref<object> const &o) noexcept
    {
      data = o.data;
    }

    void reset(T * const o) noexcept
    {
      data = o->base;
    }

    void reset(oref<T> const &o) noexcept
    {
      data = o.data;
    }

    T *operator->() const noexcept
    {
      /* TODO: Add type name. */
      //jank_assert_fmt(*this, "Null reference on oref<{}>", jtl::type_name<T>());
      jank_assert(is_some());
      return reinterpret_cast<T *>(data);
    }

    T &operator*() const noexcept
    {
      //jank_assert_fmt(*this, "Null reference on oref<{}>", jtl::type_name<T>());
      jank_assert(is_some());
      return *reinterpret_cast<T *>(data);
    }

    bool operator==(oref<object> const &rhs) const
    {
      return erase().data == rhs;
    }

    bool operator!=(oref<object> const &rhs) const
    {
      return erase().data != rhs;
    }

    template <typename C>
    requires behavior::object_like<C>
    bool operator==(oref<C> const &rhs) const
    {
      return data == rhs.data;
    }

    template <typename C>
    requires behavior::object_like<C>
    bool operator!=(oref<C> const &rhs) const
    {
      return data != rhs.data;
    }

    oref &operator=(oref const &rhs) noexcept = default;
    oref &operator=(oref &&rhs) noexcept = default;

    oref &operator=(std::remove_cv_t<std::decay_t<T>> * const rhs) noexcept
    {
      if(data == rhs)
      {
        return *this;
      }

      data = rhs;
      jank_assert(data);
      return *this;
    }

    oref &operator=(std::remove_cv_t<std::decay_t<T>> const * const rhs) noexcept
    {
      if(data == rhs)
      {
        return *this;
      }

      data = const_cast<T *>(rhs);
      jank_assert(data);
      return *this;
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    oref &operator=(oref<C> const &) noexcept
    {
      if(is_nil())
      {
        return *this;
      }

      data = std::bit_cast<void *>(jank_const_nil());
      return *this;
    }

    oref &operator=(jtl::nullptr_t) noexcept = delete;
    bool operator==(jtl::nullptr_t) noexcept = delete;
    bool operator!=(jtl::nullptr_t) noexcept = delete;

    object *get() const noexcept
    {
      return &reinterpret_cast<T *>(data)->base;
    }

    oref<object> erase() const noexcept
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

    void *data{ std::bit_cast<void *>(jank_const_nil()) };
  };

  template <>
  struct oref<obj::nil>
  {
    using value_type = obj::nil;

    oref() = default;
    oref(oref const &) = default;
    oref(oref &&) noexcept = default;

    oref(nullptr_t) = delete;

    oref(value_type * const data) noexcept
      : data{ data }
    {
      jank_assert(this->data);
    }

    oref(value_type const * const data) noexcept
      : data{ const_cast<value_type *>(data) }
    {
      jank_assert(this->data);
    }

    template <typename C>
    requires jtl::is_convertible<C *, value_type *>
    oref(oref<C> const &data) noexcept
      : data{ data.data }
    {
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    oref(oref<C> const &data) noexcept
      : data{ data.data }
    {
    }

    void reset()
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

    bool operator==(oref<object> const &rhs) const
    {
      return rhs.is_nil();
    }

    bool operator!=(oref<object> const &rhs) const
    {
      return rhs.is_some();
    }

    template <typename C>
    requires behavior::object_like<C>
    bool operator==(oref<C> const &rhs) const
    {
      return rhs.is_nil();
    }

    template <typename C>
    requires behavior::object_like<C>
    bool operator!=(oref<C> const &rhs) const
    {
      return rhs.is_some();
    }

    oref &operator=(oref const &rhs) noexcept = default;
    oref &operator=(oref &&rhs) noexcept = default;

    oref &operator=(jtl::nullptr_t) noexcept = delete;
    bool operator==(jtl::nullptr_t) noexcept = delete;
    bool operator!=(jtl::nullptr_t) noexcept = delete;

    object *get() const noexcept
    {
      return std::bit_cast<object *>(data);
    }

    oref<object> erase() const noexcept
    {
      return { std::bit_cast<object *>(jank_const_nil()) };
    }

    bool is_some() const noexcept
    {
      return false;
    }

    bool is_nil() const noexcept
    {
      return true;
    }

    value_type *data{ std::bit_cast<value_type *>(jank_const_nil()) };
  };

  template <typename T>
  jtl::ref<T> make_box(jtl::ref<T> const &o)
  {
    static_assert(sizeof(jtl::ref<T>) == sizeof(T *));
    return o;
  }

  template <typename T>
  oref<T> make_box(oref<T> const &o)
  {
    static_assert(sizeof(oref<T>) == sizeof(T *));
    return o;
  }

  /* TODO:  these. */
  template <typename T, typename... Args>
  jtl::ref<T> make_box(Args &&...args)
  {
    static_assert(sizeof(jtl::ref<T>) == sizeof(T *));
    /* TODO: Figure out cleanup for this. */
    T *ret{ new(GC) T{ std::forward<Args>(args)... } };
    if(!ret)
    {
      throw std::runtime_error{ "unable to allocate box" };
    }
    return ret;
  }

  template <typename T>
  requires(T::obj_type == object_type::boolean)
  oref<T> make_box(bool const b)
  {
    return b ? jank_true : jank_false;
  }

  template <typename T, typename... Args>
  requires behavior::object_like<T>
  oref<T> make_box(Args &&...args)
  {
    static_assert(sizeof(oref<T>) == sizeof(T *));
    oref<T> ret{ new(GC) T{ std::forward<Args>(args)... } };
    return ret;
  }

  template <typename T, usize N>
  jtl::ref<T> make_array_box()
  {
    /* TODO: Figure out cleanup for this. */
    auto const ret(new(GC) T[N]{});
    if(!ret)
    {
      throw std::runtime_error{ "unable to allocate array box" };
    }
    return ret;
  }

  template <typename T>
  jtl::ref<T> make_array_box(usize const length)
  {
    /* TODO: Figure out cleanup for this. */
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
    /* TODO: Figure out cleanup for this. */
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
