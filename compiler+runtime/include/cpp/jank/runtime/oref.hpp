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
    using small_real_ref = oref<small_real>;
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
   * can either hold a pointer value, an inline integer, or an inline real. This is done via a
   * common strategy called NaN boxing, which allows us to avoid allocations for these values.
   * NaN boxing applies to all floating point values, but only for integers up to 32 bits. Larger
   * integers will be heap allocated.
   *
   * This is an incredible performance win for numeric-heavy jank programs, since
   * it allows us to skip GC allocations for those values. However, it means that we need to
   * treat `object_ref` specially, since it isn't just a simple pointer wrapper.
   *
   * There is an excellent, detailed explanation of NaN boxing here:
   * https://www.npopov.com/2012/02/02/Pointer-magic-for-efficient-dynamic-value-representations.html
   *
   * There is a specialization of `oref` for `small_integer` and `small_real`,
   * both of which simulate a pointer to a typed object like `integer_ref` or `real_ref`, but they
   * stores the numeric value inline. This is used when we visit an `object_ref` which holds an
   * inline integer or real and we need a fully typed object.
   *
   * The GC has been configured to mask pointers such that the top 0xFFFF is ignored. This allows
   * us to keep our pointers in NaN space while still having conservative scanning work.
   *
   * It's important to note that every pointer within an `oref` is expected to be in NaN space.
   * If you try to create an `oref` with a normal pointer, you will end up with undefined
   * behavior. To mark your normal pointers as needing shifting, use the `untagged` helper. */

  struct object;

  namespace detail
  {
    /* Anything below this is an encoded real value. Anything equal to or above this is NaN. */
    constexpr u64 max_real{ 0xFFF8'0000'0000'0000ull };

    /* Integers are tagged with the high bits being 0xFFF9. The lower 32 bits store the integer.
     *                     seeeeeee|eeeemmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm
     * 0xfff9000000000000: 11111111|11111001|00000000|00000000|iiiiiiii|iiiiiiii|iiiiiiii|iiiiiiii
     */
    constexpr u64 integer_nan_tag{ 0xFFF9'0000'0000'0000ull };
    constexpr u64 integer_value_mask{ 0x0000'0000'FFFF'FFFFull };

    /* Pointers are tagged with the high bits being 0xFFFa. The lower 48 bits store the pointer.
     *                     seeeeeee|eeeemmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm
     * 0xfffa000000000000: 11111111|11111010|pppppppp|pppppppp|pppppppp|pppppppp|pppppppp|pppppppp
     */
    constexpr u64 pointer_nan_tag{ 0xFFFa'0000'0000'0000ull };
    constexpr u64 pointer_value_mask{ 0x0000'FFFF'FFFF'FFFFull };

    constexpr i32 min_small_integer{ std::numeric_limits<i32>::min() };
    constexpr i32 max_small_integer{ std::numeric_limits<i32>::max() };

    inline bool is_tagged_small_real(void * const val)
    {
      auto const bits{ std::bit_cast<u64>(val) };
      return bits < max_real;
    }

    inline bool is_tagged_small_int(void * const val)
    {
      return (std::bit_cast<u64>(val) & ~integer_value_mask) == integer_nan_tag;
    }

    inline bool is_tagged_pointer(void * const val)
    {
      return (std::bit_cast<u64>(val) & ~pointer_value_mask) == pointer_nan_tag;
    }

    inline f64 as_real(void * const val)
    {
      jank_debug_assert(is_tagged_small_real(val));
      return std::bit_cast<f64>(val);
    }

    inline i32 as_integer(void * const val)
    {
      jank_debug_assert(is_tagged_small_int(val));
      return static_cast<i32>(std::bit_cast<u64>(val) & integer_value_mask);
    }

    template <typename T>
    T *as_pointer(T * const val)
    {
      jank_debug_assert(is_tagged_pointer(val));
      return std::bit_cast<T *>(std::bit_cast<u64>(val) & pointer_value_mask);
    }

    /* This type, and the `untagged` helper functions, are used when constructing an `oref` from
     * a pointer which is not yet shifted into NaN space. This is especially common when using
     * `this` to get back to an `oref`. The type information of `untagged_ptr` will select an
     * `oref` constructor which will do the necessary NaN space shifting. */
    struct untagged_ptr
    {
      untagged_ptr() = default;

      untagged_ptr(object *data)
        : data{ data }
      {
      }

      untagged_ptr(void *data)
        : data{ reinterpret_cast<object *>(data) }
      {
      }

      object *data{};
    };

    inline untagged_ptr untagged(object const *data)
    {
      return const_cast<object *>(data);
    }

    inline untagged_ptr untagged(void *data)
    {
      return data;
    }

    template <typename T>
    requires behavior::object_like<T>
    untagged_ptr untagged(T *data)
    {
      return { static_cast<object *>(const_cast<jtl::remove_const_t<T> *>(data)) };
    }

    template <typename T>
    T tag(i32 const val)
    {
      return std::bit_cast<T>(integer_nan_tag
                              | (static_cast<u64>(static_cast<u32>(val)) & integer_value_mask));
    }

    template <typename T>
    T tag(f64 const d)
    {
      return std::bit_cast<T>(d);
    }

    template <typename T>
    T tag(untagged_ptr const ptr)
    {
      auto const val{ reinterpret_cast<u64>(ptr.data) };
      /* Our pointers are expected to fit within 48 bits. */
      //jank_debug_assert((val & ~pointer_value_mask) == 0);
      /* Our pointers are expected to have all low bits unset. */
      jank_debug_assert((val & 0b111) == 0);
      return reinterpret_cast<T>(pointer_nan_tag | val);
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
    }

    oref(value_type const * const data) noexcept
      : data{ const_cast<value_type *>(data) }
    {
    }

    /* We use this one during codegen. */
    oref(void * const data) noexcept
      : data{ static_cast<value_type *>(data) }
    {
    }

    template <typename T>
    requires behavior::object_like<T>
    oref(T * const typed_data) noexcept
      : data{ typed_data }
    {
    }

    oref(detail::untagged_ptr const p) noexcept
      : data{ detail::tag<object *>(p.data) }
    {
    }

    template <typename T>
    requires behavior::object_like<T>
    oref(T const * const typed_data) noexcept
      : data{ const_cast<T *>(typed_data) }
    {
    }

    template <typename T>
    requires(behavior::object_like<T> && !jtl::is_any_same<T, obj::small_integer, obj::small_real>)
    oref(oref<T> const &typed_data) noexcept
      : data{ typed_data.erase().raw() }
    {
    }

    template <typename T>
    requires(jtl::is_same<T, obj::small_integer>)
    oref(oref<T> const &typed_data) noexcept
      : data{ detail::tag<object *>(typed_data.data) }
    {
    }

    template <typename T>
    requires(jtl::is_same<T, obj::small_real>)
    oref(oref<T> const &typed_data) noexcept
      : data{ detail::tag<object *>(typed_data.data) }
    {
    }

    ~oref() = default;

    void reset() noexcept
    {
      data = detail::tag<object *>(std::bit_cast<object *>(&_jank_nil));
    }

    void reset(object * const o) noexcept
    {
      data = o;
    }

    void reset(detail::untagged_ptr const o) noexcept
    {
      data = detail::tag<object *>(o.data);
    }

    void reset(oref<object> const &o) noexcept
    {
      data = o.raw();
    }

    oref &operator=(oref const &rhs) noexcept = default;
    oref &operator=(oref &&rhs) noexcept = default;

    template <typename T>
    requires behavior::object_like<T>
    oref &operator=(oref<T> const &rhs) noexcept
    {
      if(data == rhs.erase().raw())
      {
        return *this;
      }

      data = rhs.erase().raw();
      return *this;
    }

    bool operator==(oref const &rhs) const noexcept
    {
      return data == rhs.raw();
    }

    template <typename T>
    requires behavior::object_like<T>
    bool operator==(oref<T> const &rhs) const noexcept
    {
      return data == rhs.erase().raw();
    }

    bool operator!=(oref const &rhs) const noexcept
    {
      return data != rhs.raw();
    }

    template <typename T>
    requires behavior::object_like<T>
    bool operator!=(oref<T> const &rhs) const noexcept
    {
      return data != rhs.erase().raw();
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
      if(detail::is_tagged_small_int(data))
      {
        return true;
      }
      if(detail::is_tagged_small_real(data))
      {
        return true;
      }

      /* NOLINTNEXTLINE(clang-analyzer-core.NullDereference): I cannot see how this can happen. We initialize to non-null and always ensure non-null on mutation. That's the whole point of this type. */
      return ptr()->type != object_type::nil;
    }

    bool is_nil() const noexcept
    {
      if(detail::is_tagged_small_int(data) || detail::is_tagged_small_real(data))
      {
        return false;
      }

      return ptr()->type == object_type::nil;
    }

    /* object_like */
    object_type get_type() const
    {
      if(detail::is_tagged_small_int(data))
      {
        return object_type::small_integer;
      }
      if(detail::is_tagged_small_real(data))
      {
        return object_type::small_real;
      }

      return ptr()->type;
    }

    bool equal(object_ref const o) const
    {
      if(detail::is_tagged_small_int(data))
      {
        if(detail::is_tagged_small_int(o.raw()))
        {
          return detail::as_integer(data) == detail::as_integer(o.raw());
        }

        obj::small_integer const i{ detail::as_integer(data) };
        return o.equal(detail::untagged(&i));
      }
      else if(detail::is_tagged_small_int(o.raw()))
      {
        obj::small_integer const i{ detail::as_integer(o.raw()) };
        return ptr()->equal(i);
      }

      if(detail::is_tagged_small_real(data))
      {
        if(detail::is_tagged_small_real(o.raw()))
        {
          return detail::as_real(data) == detail::as_real(o.raw());
        }

        obj::small_real const i{ detail::as_real(data) };
        return o.equal(detail::untagged(&i));
      }
      else if(detail::is_tagged_small_real(o.raw()))
      {
        obj::small_real const i{ detail::as_real(o.raw()) };
        return ptr()->equal(i);
      }

      return ptr()->equal(*o.ptr());
    }

    jtl::immutable_string to_string() const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.to_string();
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.to_string();
      }

      return ptr()->to_string();
    }

    void to_string(jtl::string_builder &sb) const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        i.to_string(sb);
        return;
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        i.to_string(sb);
        return;
      }

      ptr()->to_string(sb);
    }

    jtl::immutable_string to_code_string() const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.to_code_string();
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.to_code_string();
      }

      return ptr()->to_code_string();
    }

    uhash to_hash() const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.to_hash();
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.to_hash();
      }

      return ptr()->to_hash();
    }

    bool has_behavior(object_behavior const b) const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.has_behavior(b);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.has_behavior(b);
      }

      return ptr()->has_behavior(b);
    }

    /* behavior::call */
    object_ref call() const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.call();
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.call();
      }

      return ptr()->call();
    }

    object_ref call(object_ref const a1) const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.call(a1);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.call(a1);
      }

      return ptr()->call(a1);
    }

    object_ref call(object_ref const a1, object_ref const a2) const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.call(a1, a2);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.call(a1, a2);
      }

      return ptr()->call(a1, a2);
    }

    object_ref call(object_ref const a1, object_ref const a2, object_ref const a3) const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.call(a1, a2, a3);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.call(a1, a2, a3);
      }

      return ptr()->call(a1, a2, a3);
    }

    object_ref
    call(object_ref const a1, object_ref const a2, object_ref const a3, object_ref const a4) const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.call(a1, a2, a3, a4);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.call(a1, a2, a3, a4);
      }

      return ptr()->call(a1, a2, a3, a4);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5) const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.call(a1, a2, a3, a4, a5);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.call(a1, a2, a3, a4, a5);
      }

      return ptr()->call(a1, a2, a3, a4, a5);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6) const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.call(a1, a2, a3, a4, a5, a6);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.call(a1, a2, a3, a4, a5, a6);
      }

      return ptr()->call(a1, a2, a3, a4, a5, a6);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7) const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.call(a1, a2, a3, a4, a5, a6, a7);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.call(a1, a2, a3, a4, a5, a6, a7);
      }

      return ptr()->call(a1, a2, a3, a4, a5, a6, a7);
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
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.call(a1, a2, a3, a4, a5, a6, a7, a8);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.call(a1, a2, a3, a4, a5, a6, a7, a8);
      }

      return ptr()->call(a1, a2, a3, a4, a5, a6, a7, a8);
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
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.call(a1, a2, a3, a4, a5, a6, a7, a8, a9);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.call(a1, a2, a3, a4, a5, a6, a7, a8, a9);
      }

      return ptr()->call(a1, a2, a3, a4, a5, a6, a7, a8, a9);
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
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
      }

      return ptr()->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
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
                    object_ref const a10,
                    object_ref const more) const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, more);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, more);
      }

      return ptr()->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, more);
    }

    /* behavior::get */
    object_ref get(object_ref const key) const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.get(key);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.get(key);
      }

      return ptr()->get(key);
    }

    object_ref get(object_ref const key, object_ref const fallback) const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.get(key, fallback);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.get(key, fallback);
      }

      return ptr()->get(key, fallback);
    }

    bool contains(object_ref const key) const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.contains(key);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.contains(key);
      }

      return ptr()->contains(key);
    }

    /* behavior::find */
    object_ref find(object_ref key) const
    {
      if(detail::is_tagged_small_int(data))
      {
        obj::small_integer const i{ detail::as_integer(data) };
        return i.find(key);
      }
      if(detail::is_tagged_small_real(data))
      {
        obj::small_real const i{ detail::as_real(data) };
        return i.find(key);
      }

      return ptr()->find(key);
    }

    /* behavior::compare */
    i64 compare(object_ref const o) const
    {
      if(detail::is_tagged_small_int(data))
      {
        if(detail::is_tagged_small_int(o.raw()))
        {
          auto const l{ detail::as_integer(data) };
          auto const r{ detail::as_integer(o.raw()) };
          return (r < l) - (l < r);
        }

        obj::small_integer const i{ detail::as_integer(data) };
        return o.compare(detail::untagged(&i));
      }
      else if(detail::is_tagged_small_int(o.raw()))
      {
        obj::small_integer const i{ detail::as_integer(o.raw()) };
        return ptr()->compare(i);
      }

      if(detail::is_tagged_small_real(data))
      {
        if(detail::is_tagged_small_real(o.raw()))
        {
          auto const l{ detail::as_real(data) };
          auto const r{ detail::as_real(o.raw()) };
          return (r < l) - (l < r);
        }

        obj::small_real const i{ detail::as_real(data) };
        return o.compare(detail::untagged(&i));
      }
      else if(detail::is_tagged_small_real(o.raw()))
      {
        obj::small_real const i{ detail::as_real(o.raw()) };
        return ptr()->compare(i);
      }

      return ptr()->compare(*o.ptr());
    }

    /* behavior::number_like */
    i64 to_integer() const
    {
      if(detail::is_tagged_small_int(data))
      {
        return detail::as_integer(data);
      }
      if(detail::is_tagged_small_real(data))
      {
        return static_cast<i64>(detail::as_real(data));
      }

      return ptr()->to_integer();
    }

    f64 to_real() const
    {
      if(detail::is_tagged_small_int(data))
      {
        return static_cast<f64>(detail::as_integer(data));
      }
      if(detail::is_tagged_small_real(data))
      {
        return detail::as_real(data);
      }

      return ptr()->to_real();
    }

    value_type *raw() const
    {
      return data;
    }

    value_type *ptr() const
    {
      return detail::as_pointer(data);
    }

  private:
    value_type *data{ detail::tag<object *>(std::bit_cast<object *>(&_jank_nil)) };
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
      jank_debug_assert(this->data);
      jank_debug_assert(!detail::is_tagged_pointer(this->data));
    }

    oref(T const * const data) noexcept
      : data{ const_cast<T *>(data) }
    {
      jank_debug_assert(this->data);
      jank_debug_assert(!detail::is_tagged_pointer(this->data));
    }

    /* We use this one during codegen. */
    oref(void * const data) noexcept
      : data{ static_cast<T *>(data) }
    {
      jank_debug_assert(this->data);
      jank_debug_assert(!detail::is_tagged_pointer(this->data));
    }

    oref(detail::untagged_ptr const p) noexcept
      : data{ p.data }
    {
      jank_debug_assert(this->data);
      jank_debug_assert(!detail::is_tagged_pointer(this->data));
    }

    template <typename C>
    requires jtl::is_convertible<C *, T *>
    oref(oref<C> const &data) noexcept
      : data{ data.ptr() }
    {
      jank_debug_assert(!detail::is_tagged_pointer(this->data));
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    oref(oref<C> const &data) noexcept
      : data{ data.ptr() }
    {
      jank_debug_assert(!detail::is_tagged_pointer(this->data));
    }

    ~oref() = default;

    void reset() noexcept
    {
      data = std::bit_cast<object *>(&_jank_nil);
      jank_debug_assert(!detail::is_tagged_pointer(this->data));
    }

    void reset(object * const o) noexcept
    {
      data = o;
      jank_debug_assert(!detail::is_tagged_pointer(this->data));
    }

    void reset(oref<object> const &o) noexcept
    {
      data = o.ptr();
      jank_debug_assert(!detail::is_tagged_pointer(this->data));
    }

    void reset(T * const o) noexcept
    {
      data = o;
      jank_debug_assert(!detail::is_tagged_pointer(this->data));
    }

    void reset(oref<T> const &o) noexcept
    {
      data = o.ptr();
      jank_debug_assert(!detail::is_tagged_pointer(this->data));
    }

    T *operator->() const noexcept
    {
      /* TODO: Add type name. */
      //jank_debug_assert_fmt(*this, "Null reference on oref<{}>", jtl::type_name<T>());
      jank_debug_assert(is_some());

      return reinterpret_cast<T *>(data);
    }

    T &operator*() const noexcept
    {
      //jank_debug_assert_fmt(*this, "Null reference on oref<{}>", jtl::type_name<T>());
      jank_debug_assert(is_some());

      return *reinterpret_cast<T *>(data);
    }

    bool operator==(oref<object> const &rhs) const
    {
      return erase() == rhs;
    }

    bool operator!=(oref<object> const &rhs) const
    {
      return erase() != rhs;
    }

    template <typename C>
    requires behavior::object_like<C>
    bool operator==(oref<C> const &rhs) const
    {
      return data == rhs.ptr();
    }

    template <typename C>
    requires behavior::object_like<C>
    bool operator!=(oref<C> const &rhs) const
    {
      return data != rhs.ptr();
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
      jank_debug_assert(data);
      return *this;
    }

    oref &operator=(std::remove_cv_t<std::decay_t<T>> const * const rhs) noexcept
    {
      if(data == rhs)
      {
        return *this;
      }

      data = const_cast<T *>(rhs);
      jank_debug_assert(data);
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

    /* TODO: Remove these get() fns. */
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
      jank_debug_assert(ptr() != nullptr);
      return detail::untagged(static_cast<object *>(static_cast<T *>(data)));
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
      if(detail::is_tagged_small_int(o.raw()))
      {
        obj::small_integer const i{ detail::as_integer(o.raw()) };
        return static_cast<T *>(data)->equal(i);
      }
      if(detail::is_tagged_small_real(o.raw()))
      {
        obj::small_real const i{ detail::as_real(o.raw()) };
        return static_cast<T *>(data)->equal(i);
      }
      return static_cast<T *>(data)->equal(*o.ptr());
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

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8,
                    object_ref const a9,
                    object_ref const a10,
                    object_ref const more) const
    {
      if(is_nil())
      {
        return _jank_nil.call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, more);
      }
      return static_cast<T *>(data)->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, more);
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
        return _jank_nil.compare(*o.ptr());
      }
      if(detail::is_tagged_small_int(o.raw()))
      {
        obj::small_integer const i{ detail::as_integer(o.raw()) };
        return static_cast<T *>(data)->compare(i);
      }
      return static_cast<T *>(data)->compare(*o.ptr());
    }

    /* behavior::number_like */
    i64 to_integer() const
    {
      if(is_nil())
      {
        return _jank_nil.to_integer();
      }
      else if(detail::is_tagged_small_int(data))
      {
        return detail::as_integer(data);
      }
      if(detail::is_tagged_small_real(data))
      {
        return static_cast<i64>(detail::as_real(data));
      }

      return static_cast<T *>(data)->to_integer();
    }

    f64 to_real() const
    {
      if(is_nil())
      {
        return _jank_nil.to_real();
      }
      else if(detail::is_tagged_small_int(data))
      {
        return static_cast<f64>(detail::as_integer(data));
      }
      if(detail::is_tagged_small_real(data))
      {
        return detail::as_real(data);
      }

      return static_cast<T *>(data)->to_real();
    }

    void *raw() const
    {
      return data;
    }

    value_type *ptr() const
    {
      return reinterpret_cast<value_type *>(data);
    }

  private:
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
      : data{ detail::as_integer(data) }
    {
    }

    oref(i32 const data) noexcept
      : data{ data }
    {
    }

    oref(i64 const data) noexcept
      : data{ static_cast<i32>(data) }
    {
    }

    ~oref() = default;

    bool operator==(oref<object> const &rhs) const
    {
      if(detail::is_tagged_small_int(rhs.raw()))
      {
        return data == detail::as_integer(rhs.raw());
      }
      return false;
    }

    bool operator!=(oref<object> const &rhs) const
    {
      if(detail::is_tagged_small_int(rhs.raw()))
      {
        return data != detail::as_integer(rhs.raw());
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
      return detail::tag<object *>(data);
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
      if(detail::is_tagged_small_int(o.raw()))
      {
        return data == detail::as_integer(o.raw());
      }

      obj::small_integer const i{ data };
      return o.equal(detail::untagged(&i));
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

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8,
                    object_ref const a9,
                    object_ref const a10,
                    object_ref const more) const
    {
      obj::small_integer const i{ data };
      return i.call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, more);
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
      if(detail::is_tagged_small_int(o.raw()))
      {
        auto const l{ data };
        auto const r{ detail::as_integer(o.raw()) };
        return (r < l) - (l < r);
      }

      obj::small_integer const i{ data };
      return i.compare(*o.ptr());
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

    i32 raw() const
    {
      return data;
    }

    i32 data{};
  };

  template <>
  struct oref<obj::small_real>
  {
    using T = obj::small_real;
    using value_type = T;

    oref() = default;
    oref(oref const &rhs) noexcept = default;
    oref(oref &&rhs) noexcept = default;

    oref(nullptr_t) = delete;

    oref(_jank_null) noexcept
    {
    }

    oref(void * const data) noexcept
      : data{ detail::as_real(data) }
    {
    }

    /* This is used during codegen, when a floating point value can be rendered as an int. */
    oref(int const data) noexcept
      : data{ static_cast<f64>(data) }
    {
    }

    oref(f64 const data) noexcept
      : data{ data }
    {
    }

    ~oref() = default;

    bool operator==(oref<object> const &rhs) const
    {
      if(detail::is_tagged_small_real(rhs.raw()))
      {
        return data == detail::as_real(rhs.raw());
      }
      return false;
    }

    bool operator!=(oref<object> const &rhs) const
    {
      if(detail::is_tagged_small_real(rhs.raw()))
      {
        return data != detail::as_real(rhs.raw());
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
      return detail::tag<object *>(data);
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
      return object_type::small_real;
    }

    bool equal(object_ref const o) const
    {
      if(detail::is_tagged_small_real(o.raw()))
      {
        return data == detail::as_real(o.raw());
      }

      obj::small_real const i{ data };
      return o.equal(detail::untagged(&i));
    }

    jtl::immutable_string to_string() const
    {
      obj::small_real const i{ data };
      return i.to_string();
    }

    void to_string(jtl::string_builder &sb) const
    {
      obj::small_real const i{ data };
      i.to_string(sb);
    }

    jtl::immutable_string to_code_string() const
    {
      obj::small_real const i{ data };
      return i.to_code_string();
    }

    uhash to_hash() const
    {
      obj::small_real const i{ data };
      return i.to_hash();
    }

    bool has_behavior(object_behavior const b) const
    {
      obj::small_real const i{ data };
      return i.has_behavior(b);
    }

    /* behavior::call */
    object_ref call() const
    {
      obj::small_real const i{ data };
      return i.call();
    }

    object_ref call(object_ref const a1) const
    {
      obj::small_real const i{ data };
      return i.call(a1);
    }

    object_ref call(object_ref const a1, object_ref const a2) const
    {
      obj::small_real const i{ data };
      return i.call(a1, a2);
    }

    object_ref call(object_ref const a1, object_ref const a2, object_ref const a3) const
    {
      obj::small_real const i{ data };
      return i.call(a1, a2, a3);
    }

    object_ref
    call(object_ref const a1, object_ref const a2, object_ref const a3, object_ref const a4) const
    {
      obj::small_real const i{ data };
      return i.call(a1, a2, a3, a4);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5) const
    {
      obj::small_real const i{ data };
      return i.call(a1, a2, a3, a4, a5);
    }

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6) const
    {
      obj::small_real const i{ data };
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
      obj::small_real const i{ data };
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
      obj::small_real const i{ data };
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
      obj::small_real const i{ data };
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
      obj::small_real const i{ data };
      return i.call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
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
                    object_ref const a10,
                    object_ref const more) const
    {
      obj::small_real const i{ data };
      return i.call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, more);
    }

    /* behavior::get */
    object_ref get(object_ref const key) const
    {
      obj::small_real const i{ data };
      return i.get(key);
    }

    object_ref get(object_ref const key, object_ref const fallback) const
    {
      obj::small_real const i{ data };
      return i.get(key, fallback);
    }

    bool contains(object_ref const key) const
    {
      obj::small_real const i{ data };
      return i.contains(key);
    }

    /* behavior::find */
    object_ref find(object_ref key) const
    {
      obj::small_real const i{ data };
      return i.find(key);
    }

    /* behavior::compare */
    i64 compare(object_ref const o) const
    {
      if(detail::is_tagged_small_real(o.raw()))
      {
        auto const l{ data };
        auto const r{ detail::as_real(o.raw()) };
        return (r < l) - (l < r);
      }

      obj::small_real const i{ data };
      return i.compare(*o.ptr());
    }

    /* behavior::number_like */
    i64 to_integer() const
    {
      return static_cast<i64>(data);
    }

    f64 to_real() const
    {
      return data;
    }

    f64 raw() const
    {
      return data;
    }

    f64 data{};
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
      jank_debug_assert(this->data);
    }

    oref(value_type const * const data) noexcept
      : data{ const_cast<value_type *>(data) }
    {
      jank_debug_assert(this->data);
    }

    /* We use this one during codegen. */
    oref(void * const data) noexcept
      : data{ static_cast<value_type *>(data) }
    {
      jank_debug_assert(this->data);
    }

    oref(detail::untagged_ptr const p) noexcept
      : data{ reinterpret_cast<value_type *>(p.data) }
    {
      jank_debug_assert(this->data);
    }

    template <typename C>
    requires jtl::is_convertible<C *, value_type *>
    oref(oref<C> const &data) noexcept
      : data{ data.raw() }
    {
    }

    template <typename C>
    requires(C::obj_type == object_type::nil)
    oref(oref<C> const &data) noexcept
      : data{ data.raw() }
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
      return detail::untagged(std::bit_cast<object *>(&_jank_nil));
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

    object_ref call(object_ref const a1,
                    object_ref const a2,
                    object_ref const a3,
                    object_ref const a4,
                    object_ref const a5,
                    object_ref const a6,
                    object_ref const a7,
                    object_ref const a8,
                    object_ref const a9,
                    object_ref const a10,
                    object_ref const more) const
    {
      return _jank_nil.call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, more);
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
      if(detail::is_tagged_small_int(o.raw()))
      {
        obj::small_integer const i{ detail::as_integer(o.raw()) };
        return _jank_nil.compare(i);
      }
      return _jank_nil.compare(*o.ptr());
    }

    /* behavior::number_like */
    i64 to_integer() const
    {
      return _jank_nil.to_integer();
    }

    f64 to_real() const
    {
      return _jank_nil.to_real();
    }

    value_type *raw() const
    {
      return data;
    }

    value_type *ptr() const
    {
      return data;
    }

  private:
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
    if constexpr(requires { T::gc_descriptor; })
    {
      ret = reinterpret_cast<T *>(GC_malloc_explicitly_typed(sizeof(T), T::gc_descriptor));
      new(ret) T{ std::forward<Args>(args)... };
    }
    else if constexpr(requires { T::pointer_free; })
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
    static_assert(static_cast<object *>(static_cast<T *>(nullptr)) == nullptr);

    if constexpr(requires { T::gc_descriptor; })
    {
      auto const ret{ reinterpret_cast<T *>(
        GC_malloc_explicitly_typed(sizeof(T), T::gc_descriptor)) };
      new(ret) T{ std::forward<Args>(args)... };
      return detail::untagged(ret);
    }
    else if constexpr(requires { T::pointer_free; })
    {
      if constexpr(T::pointer_free)
      {
        return detail::untagged(new(PointerFreeGC) T{ std::forward<Args>(args)... });
      }
      else
      {
        return detail::untagged(new(UseGC) T{ std::forward<Args>(args)... });
      }
    }
    else
    {
      return detail::untagged(new(UseGC) T{ std::forward<Args>(args)... });
    }
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
