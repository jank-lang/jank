#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using persistent_array_map_ref = oref<struct persistent_array_map>;
  using symbol_ref = oref<struct symbol>;

  struct symbol : gc
  {
    static constexpr object_type obj_type{ object_type::symbol };
    static constexpr bool pointer_free{ false };

    symbol() = default;
    symbol(symbol &&) noexcept = default;
    symbol(symbol const &) = default;
    symbol(jtl::immutable_string const &d);
    symbol(jtl::immutable_string &&d);
    symbol(jtl::immutable_string const &ns, jtl::immutable_string const &n);
    symbol(jtl::immutable_string &&ns, jtl::immutable_string &&n);
    symbol(object_ref meta, jtl::immutable_string const &ns, jtl::immutable_string const &n);
    symbol(object_ref ns, object_ref n);

    symbol &operator=(symbol const &) = default;
    symbol &operator=(symbol &&) = default;

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::object_like extended */
    bool equal(symbol const &) const;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(symbol const &) const;

    /* behavior::metadatable */
    symbol_ref with_meta(object_ref m) const;

    /* behavior::nameable */
    jtl::immutable_string const &get_name() const;
    jtl::immutable_string const &get_namespace() const;

    bool operator==(symbol const &rhs) const;
    bool operator<(symbol const &rhs) const;

    void set_ns(jtl::immutable_string const &);
    void set_name(jtl::immutable_string const &);

    object base{ obj_type };

    /* These require mutation fns, since changing them will affect the hash. */
    jtl::immutable_string ns;
    jtl::immutable_string name;

    jtl::option<object_ref> meta;
    mutable uhash hash{};
  };
}

namespace std
{
  template <>
  struct hash<jank::runtime::obj::symbol>
  {
    size_t operator()(jank::runtime::obj::symbol const &o) const noexcept;
  };

  template <>
  struct hash<jank::runtime::obj::symbol_ref>
  {
    size_t operator()(jank::runtime::obj::symbol_ref const &o) const noexcept;
  };

  template <>
  struct equal_to<jank::runtime::obj::symbol_ref>
  {
    bool operator()(jank::runtime::obj::symbol_ref const &lhs,
                    jank::runtime::obj::symbol_ref const &rhs) const noexcept;
  };
}
