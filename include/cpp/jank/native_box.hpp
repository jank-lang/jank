#pragma once

#include <jank/runtime/object.hpp>

namespace jank
{
  /* TODO: Custom ptr in debug which checks for nullptr usage. */
  template <typename T>
  struct native_box
  {
    using value_type = T;

    native_box() = default;
    native_box(std::nullptr_t)
    { }
    native_box(std::remove_const_t<value_type> * const data)
      : data{ data }
    { }
    native_box(value_type const * const data)
      : data{ const_cast<value_type*>(data) }
    { }

    value_type* operator ->() const
    {
      assert(data);
      return data;
    }

    bool operator !() const
    { return !data; }

    value_type& operator *() const
    {
      assert(data);
      return *data;
    }

    bool operator ==(std::nullptr_t) const
    { return data == nullptr; }
    bool operator ==(native_box const &rhs) const
    { return data == rhs.data; }
    bool operator !=(std::nullptr_t) const
    { return data != nullptr; }
    bool operator !=(native_box const &rhs) const
    { return data != rhs.data; }
    bool operator <(native_box const &rhs) const
    { return data < rhs.data; }

    operator native_box<value_type const>() const
    { return data; }
    operator value_type*() const
    { return data; }
    operator runtime::object*() const
    { return &data->base; }

    explicit operator bool() const
    { return data; }

    value_type *data{};
  };

  template <>
  struct native_box<runtime::object>
  {
    using value_type = runtime::object;

    native_box() = default;
    native_box(std::nullptr_t)
    { }
    native_box(value_type * const data)
      : data{ data }
    { }
    template <runtime::object_type T>
    native_box(runtime::static_object<T> * const typed_data)
      : data{ &typed_data->base }
    { }
    template <runtime::object_type T>
    native_box(runtime::static_object<T> const * const typed_data)
      : data{ typed_data ? const_cast<runtime::object*>(&typed_data->base) : nullptr }
    { }
    template <runtime::object_type T>
    native_box(native_box<runtime::static_object<T>> const typed_data)
      : data{ typed_data ? &typed_data->base : nullptr }
    { }

    value_type* operator ->() const
    {
      assert(data);
      return data;
    }

    bool operator !() const
    { return !data; }

    value_type& operator *() const
    {
      assert(data);
      return *data;
    }

    bool operator ==(std::nullptr_t) const
    { return data == nullptr; }
    bool operator ==(native_box const &rhs) const
    { return data == rhs.data; }
    template <runtime::object_type T>
    bool operator ==(runtime::static_object<T> const &rhs) const
    { return data == &rhs->base; }
    template <runtime::object_type T>
    bool operator ==(native_box<runtime::static_object<T>> const &rhs) const
    { return data == &rhs->base; }

    bool operator !=(std::nullptr_t) const
    { return data != nullptr; }
    bool operator !=(native_box const &rhs) const
    { return data != rhs.data; }
    template <runtime::object_type T>
    bool operator !=(runtime::static_object<T> const &rhs) const
    { return data != &rhs->base; }
    template <runtime::object_type T>
    bool operator !=(native_box<runtime::static_object<T>> const &rhs) const
    { return data != &rhs->base; }

    bool operator <(native_box const &rhs) const
    { return data < rhs.data; }

    operator native_box<value_type const>() const
    { return data; }
    operator value_type*() const
    { return data; }

    explicit operator bool() const
    { return data; }

    value_type *data{};
  };


  template <typename T>
  struct remove_box
  { using type = T; };
  template <typename T>
  struct remove_box<native_box<T>>
  { using type = T; };
  template <typename T>
  using remove_box_t = typename remove_box<T>::type;

  template <typename T>
  native_box<T> make_box(native_box<T> const &o)
  { return o; }
  template <typename T, typename... Args>
  native_box<T> make_box(Args &&... args)
  {
    //static_assert(offsetof(T, base) == 0, "object base needs to be the first member of each typed object");

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
    //static_assert(offsetof(T, base) == 0, "object base needs to be the first member of each typed object");

    /* TODO: pointer_free? */
    auto const ret(new (GC) T[sizeof...(Args)]{ std::forward<Args>(args)... });
    if(!ret)
    { throw std::runtime_error{ "unable to allocate array box" }; }
    return ret;
  }
}
