#pragma once

#include <type_traits>

#include <jtl/trait/transform.hpp>
#include <jtl/ref.hpp>
#include <jtl/ptr.hpp>
#include <jtl/assert.hpp>

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/number.hpp>

/* During AOT codegen, we need to initialize a bunch of lifted constants and globals, but
 * the jank_nil global may not yet be initialized, since the order of initialization of globals
 * across C++ translation units is not defined. In the default initialization or oref, we
 * try to grab the `jank_nil` global, which will be undefined behavior. Instead, we use
 * this special tag type to initialize our orefs to nullptr, since will later do an in-place
 * new on them with a proper ctor. */
struct _jank_null
{
};

namespace jank::runtime
{
  namespace obj
  {
    struct nil;

    using boolean_ref = oref<boolean>;
    using integer_ref = oref<integer>;
    using small_integer_ref = oref<small_integer>;
    using real_ref = oref<real>;
  }

  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  extern oref<struct obj::boolean> jank_true;
  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  extern oref<struct obj::boolean> jank_false;

  /* `oref` is jank's pointer wrapper for boxed runtime objects. It is only used for objects
   * which inherit from `jank::runtime::object` and implement the necessary behaviors. It's
   * formidable, though, since it serves a few very important roles in the jank runtime.
   *
   * 1. It provides a type-erased `object_ref`, which can represent any runtime object
   *    (like Java's `Object`)
   * 2. It provides typed refs, like `integer_ref`, `persistent_string_ref`, and so on, for
   *    every runtime object type
   * 3. It allows any of these refs to be `nil` and provides helpers for querying that
   * 4. It supports inline values for certain tagged pointers
   *
   * To expand on the inline values and tagged pointers, `object_ref` (the type-erased `oref`)
   * can either hold a pointer value or an inline integer. The integer is case indicated by the
   * lowest bit of the pointer value being 1, in which case a 63 bit integer exists instead of
   * a pointer value. This is an incredible performance win for integer-heavy jank programs, since
   * it allows us to skip GC allocations for those integers. However, it means that we need to
   * treat `object_ref` specially, since it isn't just a simple pointer wrapper.
   *
   * Similarly, there is a specialization of `oref` for `small_integer`, which simulates a pointer
   * to a typed integral object like `integer_ref`, but it stores the integer value inline. This
   * is used when we visit an `object_ref` which holds an inline integer and we need a fully typed
   * object. */

  /* TODO: Support inline doubles. */

  namespace detail
  {
    constexpr char integer_tag{ 0b1 };
    constexpr char integer_tag_mask{ 0b1 };
    constexpr char integer_shift{ 1 };
    constexpr i64 max_small_integer{ std::numeric_limits<i64>::max() >> integer_shift };
    constexpr i64 min_small_integer{ std::numeric_limits<i64>::min() >> integer_shift };

    inline bool is_small_int(void const *data)
    {
      return (reinterpret_cast<int64_t>(data) & integer_tag_mask) == integer_tag;
    }

    inline i64 as_int(void const *data)
    {
      /* NOLINTNEXTLINE(cppcoreguidelines-narrowing-conversions, bugprone-narrowing-conversions) */
      return reinterpret_cast<i64>(data) >> integer_shift;
    }

    template <typename T>
    T as_ptr(i64 const data)
    {
      return reinterpret_cast<T>((data << integer_shift) | integer_tag_mask);
    }
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
    using object_ref = oref<O>;

    oref() = default;
    oref(oref const &rhs) = default;
    oref(oref &&rhs) noexcept = default;

    oref(nullptr_t) noexcept = delete;

    oref(_jank_null) noexcept
      : data{ nullptr }
    {
    }

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

    /* We use this one during codegen. */
    oref(void * const data) noexcept
      : data{ static_cast<value_type *>(data) }
    {
      jank_assert(this->data);
    }

    template <typename T>
    requires behavior::object_like<T>
    oref(T * const typed_data) noexcept
      : data{ typed_data }
    {
      jank_assert(this->data);
    }

    template <typename T>
    requires behavior::object_like<T>
    oref(T const * const typed_data) noexcept
      : data{ const_cast<T *>(typed_data) }
    {
      jank_assert(this->data);
    }

    template <typename T>
    requires(behavior::object_like<T> && !jtl::is_same<T, obj::small_integer>)
    oref(oref<T> const &typed_data) noexcept
      : data{ typed_data.erase().data }
    {
    }

    template <typename T>
    requires(jtl::is_same<T, obj::small_integer>)
    oref(oref<T> const &typed_data) noexcept
      : data{ detail::as_ptr<object *>(typed_data.data) }
    {
    }

    ~oref() = default;

    void reset() noexcept
    {
      data = std::bit_cast<object *>(&_jank_nil);
    }

    void reset(object * const o) noexcept
    {
      data = o;
    }

    void reset(oref<object> const &o) noexcept
    {
      data = o.data;
    }

    oref &operator=(oref const &rhs) noexcept = default;
    oref &operator=(oref &&rhs) noexcept = default;

    template <typename T>
    requires behavior::object_like<T>
    oref &operator=(oref<T> const &rhs) noexcept
    {
      if(data == rhs.data)
      {
        return *this;
      }

      data = rhs.get();
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

    object_ref const &erase() const noexcept
    {
      return *this;
    }

    bool is_some() const noexcept
    {
      if(detail::is_small_int(data))
      {
        return true;
      }

      /* NOLINTNEXTLINE(clang-analyzer-core.NullDereference): I cannot see how this can happen. We initialize to non-null and always ensure non-null on mutation. That's the whole point of this type. */
      return data->type != object_type::nil;
    }

    bool is_nil() const noexcept
    {
      if(detail::is_small_int(data))
      {
        return false;
      }

      return data->type == object_type::nil;
    }

    /* object_like */
    object_type get_type() const
    {
      if(detail::is_small_int(data))
      {
        return object_type::small_integer;
      }
      return data->type;
    }

    bool equal(object_ref const o) const
    {
      if(detail::is_small_int(data))
      {
        if(detail::is_small_int(o.data))
        {
          return detail::as_int(data) == detail::as_int(o.data);
        }

        obj::small_integer const i{ detail::as_int(data) };
        return o.equal(&i);
      }
      else if(detail::is_small_int(o.data))
      {
        obj::small_integer const i{ detail::as_int(o.data) };
        return data->equal(i);
      }
      return data->equal(*o.data);
    }

    jtl::immutable_string to_string() const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.to_string();
      }
      return data->to_string();
    }

    void to_string(jtl::string_builder &sb) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        i.to_string(sb);
        return;
      }
      data->to_string(sb);
    }

    jtl::immutable_string to_code_string() const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.to_code_string();
      }
      return data->to_code_string();
    }

    uhash to_hash() const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.to_hash();
      }
      return data->to_hash();
    }

    bool has_behavior(object_behavior const b) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.has_behavior(b);
      }
      return data->has_behavior(b);
    }

    /* behavior::call */
    object_ref call() const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.call();
      }
      return data->call();
    }

    object_ref call(object_ref const a1) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.call(a1);
      }
      return data->call(a1);
    }

    object_ref call(object_ref const a1, object_ref const a2) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.call(a1, a2);
      }
      return data->call(a1, a2);
    }

    object_ref call(object_ref const a1, object_ref const a2, object_ref const a3) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.call(a1, a2, a3);
      }
      return data->call(a1, a2, a3);
    }

    object_ref
    call(object_ref const a1, object_ref const a2, object_ref const a3, object_ref const a4) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.call(a1, a2, a3, a4);
      }
      return data->call(a1, a2, a3, a4);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.call(a1, a2, a3, a4, a5);
      }
      return data->call(a1, a2, a3, a4, a5);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.call(a1, a2, a3, a4, a5, a6);
      }
      return data->call(a1, a2, a3, a4, a5, a6);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.call(a1, a2, a3, a4, a5, a6, a7);
      }
      return data->call(a1, a2, a3, a4, a5, a6, a7);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.call(a1, a2, a3, a4, a5, a6, a7, a8);
      }
      return data->call(a1, a2, a3, a4, a5, a6, a7, a8);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8,
                    object_ref const a9) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.call(a1, a2, a3, a4, a5, a6, a7, a8, a9);
      }
      return data->call(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8,
                    object_ref const a9,
                    object_ref const a10) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
      }
      return data->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    }

    callable_arity_flags get_arity_flags() const
    {
      if(detail::is_small_int(data))
      {
        return 0;
      }
      return data->get_arity_flags();
    }

    /* behavior::get */
    object_ref get(object_ref const key) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.get(key);
      }
      return data->get(key);
    }

    object_ref get(object_ref const key, object_ref const fallback) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.get(key, fallback);
      }
      return data->get(key, fallback);
    }

    bool contains(object_ref const key) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.contains(key);
      }
      return data->contains(key);
    }

    /* behavior::find */
    object_ref find(object_ref key) const
    {
      if(detail::is_small_int(data))
      {
        obj::small_integer const i{ detail::as_int(data) };
        return i.find(key);
      }
      return data->find(key);
    }

    /* behavior::compare */
    i64 compare(object_ref const o) const
    {
      if(detail::is_small_int(data))
      {
        if(detail::is_small_int(o.data))
        {
          auto const l{ detail::as_int(data) };
          auto const r{ detail::as_int(o.data) };
          return (r < l) - (l < r);
        }

        obj::small_integer const i{ detail::as_int(data) };
        return o.compare(&i);
      }
      else if(detail::is_small_int(o.data))
      {
        obj::small_integer const i{ detail::as_int(o.data) };
        return data->compare(i);
      }
      return data->compare(*o.data);
    }

    value_type *data{ std::bit_cast<object *>(&_jank_nil) };
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

    oref(_jank_null) noexcept
      : data{ nullptr }
    {
    }

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

    /* We use this one during codegen. */
    oref(void * const data) noexcept
      : data{ static_cast<T *>(data) }
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
      data = std::bit_cast<object *>(&_jank_nil);
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
      data = o;
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

      static_assert(!jtl::is_same<T, obj::small_integer>,
                    "operator-> is not supported for small_integer_ref");

      return reinterpret_cast<T *>(data);
    }

    T &operator*() const noexcept
    {
      //jank_assert_fmt(*this, "Null reference on oref<{}>", jtl::type_name<T>());
      jank_assert(is_some());

      static_assert(!jtl::is_same<T, obj::small_integer>,
                    "operator* is not supported for small_integer_ref");

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

      data = std::bit_cast<void *>(&_jank_nil);
      return *this;
    }

    oref &operator=(jtl::nullptr_t) noexcept = delete;
    bool operator==(jtl::nullptr_t) noexcept = delete;
    bool operator!=(jtl::nullptr_t) noexcept = delete;

    object *get() const noexcept
    {
      return static_cast<object *>(static_cast<T *>(data));
    }

    oref<object> erase() const noexcept
    {
      if(is_nil())
      {
        return {};
      }
      return static_cast<object *>(static_cast<T *>(data));
    }

    bool is_some() const noexcept
    {
      return data != std::bit_cast<void *>(&_jank_nil);
    }

    bool is_nil() const noexcept
    {
      return data == std::bit_cast<void *>(&_jank_nil);
    }

    /* object_like */
    object_type get_type() const
    {
      if(is_nil())
      {
        return object_type::nil;
      }
      return static_cast<T *>(data)->type;
    }

    bool equal(object_ref const o) const
    {
      if(is_nil())
      {
        return o.is_nil();
      }
      if(detail::is_small_int(o.data))
      {
        obj::small_integer const i{ detail::as_int(o.data) };
        return static_cast<T *>(data)->equal(i);
      }
      return static_cast<T *>(data)->equal(*o.data);
    }

    jtl::immutable_string to_string() const
    {
      if(is_nil())
      {
        return _jank_nil.to_string();
      }
      return static_cast<T *>(data)->to_string();
    }

    void to_string(jtl::string_builder &sb) const
    {
      if(is_nil())
      {
        _jank_nil.to_string(sb);
        return;
      }
      static_cast<T *>(data)->to_string(sb);
    }

    jtl::immutable_string to_code_string() const
    {
      if(is_nil())
      {
        return _jank_nil.to_code_string();
      }
      return static_cast<T *>(data)->to_code_string();
    }

    uhash to_hash() const
    {
      if(is_nil())
      {
        return _jank_nil.to_hash();
      }
      return static_cast<T *>(data)->to_hash();
    }

    bool has_behavior(object_behavior const b) const
    {
      if(is_nil())
      {
        return _jank_nil.has_behavior(b);
      }
      return static_cast<T *>(data)->has_behavior(b);
    }

    /* behavior::call */
    object_ref call() const
    {
      if(is_nil())
      {
        return _jank_nil.call();
      }
      return static_cast<T *>(data)->call();
    }

    object_ref call(object_ref const a1) const
    {
      if(is_nil())
      {
        return _jank_nil.call(a1);
      }
      return static_cast<T *>(data)->call(a1);
    }

    object_ref call(object_ref const a1, object_ref const a2) const
    {
      if(is_nil())
      {
        return _jank_nil.call(a1, a2);
      }
      return static_cast<T *>(data)->call(a1, a2);
    }

    object_ref call(object_ref const a1, object_ref const a2, object_ref const a3) const
    {
      if(is_nil())
      {
        return _jank_nil.call(a1, a2, a3);
      }
      return static_cast<T *>(data)->call(a1, a2, a3);
    }

    object_ref
    call(object_ref const a1, object_ref const a2, object_ref const a3, object_ref const a4) const
    {
      if(is_nil())
      {
        return _jank_nil.call(a1, a2, a3, a4);
      }
      return static_cast<T *>(data)->call(a1, a2, a3, a4);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5) const
    {
      if(is_nil())
      {
        return _jank_nil.call(a1, a2, a3, a4, a5);
      }
      return static_cast<T *>(data)->call(a1, a2, a3, a4, a5);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6) const
    {
      if(is_nil())
      {
        return _jank_nil.call(a1, a2, a3, a4, a5, a6);
      }
      return static_cast<T *>(data)->call(a1, a2, a3, a4, a5, a6);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7) const
    {
      if(is_nil())
      {
        return _jank_nil.call(a1, a2, a3, a4, a5, a6, a7);
      }
      return static_cast<T *>(data)->call(a1, a2, a3, a4, a5, a6, a7);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8) const
    {
      if(is_nil())
      {
        return _jank_nil.call(a1, a2, a3, a4, a5, a6, a7, a8);
      }
      return static_cast<T *>(data)->call(a1, a2, a3, a4, a5, a6, a7, a8);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8,
                    object_ref const a9) const
    {
      if(is_nil())
      {
        return _jank_nil.call(a1, a2, a3, a4, a5, a6, a7, a8, a9);
      }
      return static_cast<T *>(data)->call(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8,
                    object_ref const a9,
                    object_ref const a10) const
    {
      if(is_nil())
      {
        return _jank_nil.call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
      }
      return static_cast<T *>(data)->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    }

    callable_arity_flags get_arity_flags() const
    {
      if(is_nil())
      {
        return _jank_nil.get_arity_flags();
      }
      return static_cast<T *>(data)->get_arity_flags();
    }

    /* behavior::get */
    object_ref get(object_ref const key) const
    {
      if(is_nil())
      {
        return _jank_nil.get(key);
      }
      return static_cast<T *>(data)->get(key);
    }

    object_ref get(object_ref const key, object_ref const fallback) const
    {
      if(is_nil())
      {
        return _jank_nil.get(key, fallback);
      }
      return static_cast<T *>(data)->get(key, fallback);
    }

    bool contains(object_ref const key) const
    {
      if(is_nil())
      {
        return _jank_nil.contains(key);
      }
      return static_cast<T *>(data)->contains(key);
    }

    /* behavior::find */
    object_ref find(object_ref key) const
    {
      if(is_nil())
      {
        return _jank_nil.find(key);
      }
      return static_cast<T *>(data)->find(key);
    }

    /* behavior::compare */
    i64 compare(object_ref const o) const
    {
      if(is_nil())
      {
        return _jank_nil.compare(*o.data);
      }
      if(detail::is_small_int(o.data))
      {
        obj::small_integer const i{ detail::as_int(o.data) };
        return static_cast<T *>(data)->compare(i);
      }
      return static_cast<T *>(data)->compare(*o.data);
    }

    void *data{ std::bit_cast<void *>(&_jank_nil) };
  };

  template <>
  struct oref<obj::small_integer>
  {
    using T = obj::small_integer;
    using value_type = T;

    oref() = default;
    oref(oref const &rhs) noexcept = default;
    oref(oref &&rhs) noexcept = default;

    oref(nullptr_t) = delete;

    oref(_jank_null) noexcept
    {
    }

    oref(void * const data) noexcept
      : data{ detail::as_int(data) }
    {
    }

    oref(i64 const data) noexcept
      : data{ data }
    {
    }

    ~oref() = default;

    bool operator==(oref<object> const &rhs) const
    {
      if(detail::is_small_int(rhs.data))
      {
        return data == detail::as_int(rhs.data);
      }
      return false;
    }

    bool operator!=(oref<object> const &rhs) const
    {
      if(detail::is_small_int(rhs.data))
      {
        return data != detail::as_int(rhs.data);
      }
      return true;
    }

    oref &operator=(oref const &rhs) noexcept = default;
    oref &operator=(oref &&rhs) noexcept = default;

    oref &operator=(jtl::nullptr_t) noexcept = delete;
    bool operator==(jtl::nullptr_t) noexcept = delete;
    bool operator!=(jtl::nullptr_t) noexcept = delete;

    oref<object> erase() const noexcept
    {
      return detail::as_ptr<object *>(data);
    }

    bool is_some() const noexcept
    {
      return true;
    }

    bool is_nil() const noexcept
    {
      return false;
    }

    oref const *operator->() const noexcept
    {
      return this;
    }

    oref const &operator*() const noexcept
    {
      return *this;
    }

    /* object_like */
    object_type get_type() const
    {
      return object_type::small_integer;
    }

    bool equal(object_ref const o) const
    {
      if(detail::is_small_int(o.data))
      {
        return data == detail::as_int(o.data);
      }

      obj::small_integer const i{ data };
      return o.equal(&i);
    }

    jtl::immutable_string to_string() const
    {
      obj::small_integer const i{ data };
      return i.to_string();
    }

    void to_string(jtl::string_builder &sb) const
    {
      obj::small_integer const i{ data };
      i.to_string(sb);
    }

    jtl::immutable_string to_code_string() const
    {
      obj::small_integer const i{ data };
      return i.to_code_string();
    }

    uhash to_hash() const
    {
      obj::small_integer const i{ data };
      return i.to_hash();
    }

    bool has_behavior(object_behavior const b) const
    {
      obj::small_integer const i{ data };
      return i.has_behavior(b);
    }

    /* behavior::call */
    object_ref call() const
    {
      obj::small_integer const i{ data };
      return i.call();
    }

    object_ref call(object_ref const a1) const
    {
      obj::small_integer const i{ data };
      return i.call(a1);
    }

    object_ref call(object_ref const a1, object_ref const a2) const
    {
      obj::small_integer const i{ data };
      return i.call(a1, a2);
    }

    object_ref call(object_ref const a1, object_ref const a2, object_ref const a3) const
    {
      obj::small_integer const i{ data };
      return i.call(a1, a2, a3);
    }

    object_ref
    call(object_ref const a1, object_ref const a2, object_ref const a3, object_ref const a4) const
    {
      obj::small_integer const i{ data };
      return i.call(a1, a2, a3, a4);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5) const
    {
      obj::small_integer const i{ data };
      return i.call(a1, a2, a3, a4, a5);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6) const
    {
      obj::small_integer const i{ data };
      return i.call(a1, a2, a3, a4, a5, a6);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7) const
    {
      obj::small_integer const i{ data };
      return i.call(a1, a2, a3, a4, a5, a6, a7);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8) const
    {
      obj::small_integer const i{ data };
      return i.call(a1, a2, a3, a4, a5, a6, a7, a8);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8,
                    object_ref const a9) const
    {
      obj::small_integer const i{ data };
      return i.call(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8,
                    object_ref const a9,
                    object_ref const a10) const
    {
      obj::small_integer const i{ data };
      return i.call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    }

    callable_arity_flags get_arity_flags() const
    {
      return 0;
    }

    /* behavior::get */
    object_ref get(object_ref const key) const
    {
      obj::small_integer const i{ data };
      return i.get(key);
    }

    object_ref get(object_ref const key, object_ref const fallback) const
    {
      obj::small_integer const i{ data };
      return i.get(key, fallback);
    }

    bool contains(object_ref const key) const
    {
      obj::small_integer const i{ data };
      return i.contains(key);
    }

    /* behavior::find */
    object_ref find(object_ref key) const
    {
      obj::small_integer const i{ data };
      return i.find(key);
    }

    /* behavior::compare */
    i64 compare(object_ref const o) const
    {
      if(detail::is_small_int(o.data))
      {
        auto const l{ data };
        auto const r{ detail::as_int(o.data) };
        return (r < l) - (l < r);
      }

      obj::small_integer const i{ data };
      return i.compare(*o.data);
    }

    /* behavior::number_like */
    i64 to_integer() const
    {
      return data;
    }

    f64 to_real() const
    {
      return static_cast<f64>(data);
    }

    i64 data{};
  };

  template <>
  struct oref<obj::nil>
  {
    using value_type = obj::nil;

    oref() = default;
    oref(oref const &) = default;
    oref(oref &&) noexcept = default;

    oref(nullptr_t) = delete;

    oref(_jank_null) noexcept
      : data{ nullptr }
    {
    }

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

    /* We use this one during codegen. */
    oref(void * const data) noexcept
      : data{ static_cast<value_type *>(data) }
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
      return { std::bit_cast<object *>(&_jank_nil) };
    }

    bool is_some() const noexcept
    {
      return false;
    }

    bool is_nil() const noexcept
    {
      return true;
    }

    /* object_like */
    object_type get_type() const
    {
      return object_type::nil;
    }

    bool equal(object_ref const o) const
    {
      return o.is_nil();
    }

    jtl::immutable_string to_string() const
    {
      return _jank_nil.to_string();
    }

    void to_string(jtl::string_builder &sb) const
    {
      _jank_nil.to_string(sb);
    }

    jtl::immutable_string to_code_string() const
    {
      return _jank_nil.to_code_string();
    }

    uhash to_hash() const
    {
      return _jank_nil.to_hash();
    }

    bool has_behavior(object_behavior const b) const
    {
      return _jank_nil.has_behavior(b);
    }

    /* behavior::call */
    object_ref call() const
    {
      return _jank_nil.call();
    }

    object_ref call(object_ref const a1) const
    {
      return _jank_nil.call(a1);
    }

    object_ref call(object_ref const a1, object_ref const a2) const
    {
      return _jank_nil.call(a1, a2);
    }

    object_ref call(object_ref const a1, object_ref const a2, object_ref const a3) const
    {
      return _jank_nil.call(a1, a2, a3);
    }

    object_ref
    call(object_ref const a1, object_ref const a2, object_ref const a3, object_ref const a4) const
    {
      return _jank_nil.call(a1, a2, a3, a4);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5) const
    {
      return _jank_nil.call(a1, a2, a3, a4, a5);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6) const
    {
      return _jank_nil.call(a1, a2, a3, a4, a5, a6);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7) const
    {
      return _jank_nil.call(a1, a2, a3, a4, a5, a6, a7);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8) const
    {
      return _jank_nil.call(a1, a2, a3, a4, a5, a6, a7, a8);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8,
                    object_ref const a9) const
    {
      return _jank_nil.call(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8,
                    object_ref const a9,
                    object_ref const a10) const
    {
      return _jank_nil.call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    }

    callable_arity_flags get_arity_flags() const
    {
      return _jank_nil.get_arity_flags();
    }

    /* behavior::get */
    object_ref get(object_ref const key) const
    {
      return _jank_nil.get(key);
    }

    object_ref get(object_ref const key, object_ref const fallback) const
    {
      return _jank_nil.get(key, fallback);
    }

    bool contains(object_ref const key) const
    {
      return _jank_nil.contains(key);
    }

    /* behavior::find */
    object_ref find(object_ref key) const
    {
      return _jank_nil.find(key);
    }

    /* behavior::compare */
    i64 compare(object_ref const o) const
    {
      if(detail::is_small_int(o.data))
      {
        obj::small_integer const i{ detail::as_int(o.data) };
        return _jank_nil.compare(i);
      }
      return _jank_nil.compare(*o.data);
    }

    value_type *data{ std::bit_cast<value_type *>(&_jank_nil) };
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

  /* TODO: constexpr these. */
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
        ret = new(UseGC) T{ std::forward<Args>(args)... };
      }
    }
    else
    {
      ret = new(UseGC) T{ std::forward<Args>(args)... };
    }
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
    oref<T> ret;
    if constexpr(requires { T::pointer_free; })
    {
      if constexpr(T::pointer_free)
      {
        ret = new(PointerFreeGC) T{ std::forward<Args>(args)... };
      }
      else
      {
        ret = new(UseGC) T{ std::forward<Args>(args)... };
      }
    }
    else
    {
      ret = new(UseGC) T{ std::forward<Args>(args)... };
    }
    return ret;
  }

  template <typename T, usize N>
  jtl::ref<T> make_array_box()
  {
    /* TODO: Figure out cleanup for this. */
    auto const ret(new(UseGC) T[N]{});
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
    auto const ret(new(UseGC) T[length]{});
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
    auto const ret(new(UseGC) T[sizeof...(Args)]{ std::forward<Args>(args)... });
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
