#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/hash.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using map = static_object<object_type::map>;
    using map_ptr = native_box<map>;
  }

  template <>
  struct static_object<object_type::symbol> : gc
  {
    static constexpr bool pointer_free{ true };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(object &&base);
    static_object(native_string const &d);
    static_object(native_string &&d);
    static_object(native_string const &ns, native_string const &n);
    static_object(native_string &&ns, native_string &&n);

    static_object& operator=(static_object const&) = default;
    static_object& operator=(static_object &&) = default;

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_integer to_hash() const;

    /* behavior::objectable extended */
    native_bool equal(static_object const &) const;

    /* behavior::metadatable */
    object_ptr with_meta(object_ptr m) const;

    bool operator ==(static_object const &rhs) const;
    bool operator <(static_object const &rhs) const;

    object base{ object_type::symbol };
    native_string ns;
    native_string name;
    option<obj::map_ptr> meta;
  };

  namespace obj
  {
    using symbol = static_object<object_type::symbol>;
    using symbol_ptr = native_box<symbol>;
  }
}

namespace std
{
  template <>
  struct hash<jank::runtime::obj::symbol>
  {
    size_t operator()(jank::runtime::obj::symbol const &o) const noexcept
    { return static_cast<size_t>(jank::runtime::detail::hash_combine(o.ns.to_hash(), o.name.to_hash())); }
  };

  template <>
  struct hash<jank::runtime::obj::symbol_ptr>
  {
    size_t operator()(jank::runtime::obj::symbol_ptr const &o) const noexcept
    {
      static auto hasher(std::hash<jank::runtime::obj::symbol>{});
      return hasher(*o);
    }
  };

  template <>
  struct equal_to<jank::runtime::obj::symbol_ptr>
  {
    bool operator()
    (
      jank::runtime::obj::symbol_ptr const &lhs,
      jank::runtime::obj::symbol_ptr const &rhs
    ) const noexcept
    {
      if(!lhs)
      { return !rhs; }
      else if(!rhs)
      { return !lhs; }
      return lhs->equal(*rhs);
    }
  };
}
