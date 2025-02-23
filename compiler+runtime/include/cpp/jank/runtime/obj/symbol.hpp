#pragma once

#include <jank/runtime/object.hpp>
#include <jank/option.hpp>

namespace jank::runtime::obj
{
  using persistent_array_map_ptr = native_box<struct persistent_array_map>;
  using symbol_ptr = native_box<struct symbol>;

  struct symbol : gc
  {
    static constexpr object_type obj_type{ object_type::symbol };
    static constexpr native_bool pointer_free{ false };

    symbol() = default;
    symbol(symbol &&) noexcept = default;
    symbol(symbol const &) = default;
    symbol(native_persistent_string const &d);
    symbol(native_persistent_string &&d);
    symbol(native_persistent_string const &ns, native_persistent_string const &n);
    symbol(native_persistent_string &&ns, native_persistent_string &&n);
    symbol(native_persistent_string const &ns, native_persistent_string const &n, object_ptr meta);
    symbol(object_ptr ns, object_ptr n);

    symbol &operator=(symbol const &) = default;
    symbol &operator=(symbol &&) = default;

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::object_like extended */
    native_bool equal(symbol const &) const;

    /* behavior::comparable */
    native_integer compare(object const &) const;

    /* behavior::comparable extended */
    native_integer compare(symbol const &) const;

    /* behavior::metadatable */
    symbol_ptr with_meta(object_ptr m) const;

    /* behavior::nameable */
    native_persistent_string const &get_name() const;
    native_persistent_string const &get_namespace() const;

    native_bool operator==(symbol const &rhs) const;
    native_bool operator<(symbol const &rhs) const;

    void set_ns(native_persistent_string const &);
    void set_name(native_persistent_string const &);

    object base{ obj_type };

    /* These require mutation fns, since changing them will affect the hash. */
    native_persistent_string ns;
    native_persistent_string name;

    option<object_ptr> meta;
    mutable native_hash hash{};
  };
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
        return false;
      }
      return lhs->equal(*rhs);
    }
  };
}
