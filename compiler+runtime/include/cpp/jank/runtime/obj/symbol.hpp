#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_array_map = static_object<object_type::persistent_array_map>;
    using persistent_array_map_ptr = native_box<persistent_array_map>;
  }

  template <>
  struct static_object<object_type::symbol> : gc
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(native_persistent_string const &d);
    static_object(native_persistent_string &&d);
    static_object(native_persistent_string const &ns, native_persistent_string const &n);
    static_object(native_persistent_string &&ns, native_persistent_string &&n);
    static_object(native_persistent_string const &ns,
                  native_persistent_string const &n,
                  object_ptr meta);

    static_object &operator=(static_object const &) = default;
    static_object &operator=(static_object &&) = default;

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    /* behavior::object_like extended */
    native_bool equal(static_object const &) const;

    /* behavior::comparable */
    native_integer compare(object const &) const;

    /* behavior::comparable extended */
    native_integer compare(static_object const &) const;

    /* behavior::metadatable */
    native_box<static_object> with_meta(object_ptr m) const;

    /* behavior::nameable */
    native_persistent_string const &get_name() const;
    native_persistent_string const &get_namespace() const;

    native_bool operator==(static_object const &rhs) const;
    native_bool operator<(static_object const &rhs) const;

    void set_ns(native_persistent_string const &);
    void set_name(native_persistent_string const &);

    object base{ object_type::symbol };

    /* These require mutation fns, since changing them will affect the hash. */
    native_persistent_string ns;
    native_persistent_string name;

    option<object_ptr> meta;
    mutable native_hash hash{};
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
    {
      return o.to_hash();
    }
  };

  template <>
  struct hash<jank::runtime::obj::symbol_ptr>
  {
    size_t operator()(jank::runtime::obj::symbol_ptr const &o) const noexcept
    {
      return o->to_hash();
    }
  };

  template <>
  struct equal_to<jank::runtime::obj::symbol_ptr>
  {
    bool operator()(jank::runtime::obj::symbol_ptr const &lhs,
                    jank::runtime::obj::symbol_ptr const &rhs) const noexcept
    {
      if(!lhs)
      {
        return !rhs;
      }
      else if(!rhs)
      {
        return !lhs;
      }
      return lhs->equal(*rhs);
    }
  };
}
