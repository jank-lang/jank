#pragma once

#include <functional>

#include <jank/runtime/object.hpp>
#include <jank/runtime/hash.hpp>

namespace jank::runtime::obj
{
  struct symbol : object, pool_item_base<symbol>
  {
    symbol() = default;
    symbol(symbol &&) = default;
    symbol(symbol const &) = default;
    symbol(detail::string_type const &d);
    symbol(detail::string_type &&d);
    symbol(detail::string_type const &ns, detail::string_type const &n)
      : ns{ ns }, name{ n }
    { }
    symbol(detail::string_type &&ns, detail::string_type &&n)
      : ns{ std::move(ns) }, name{ std::move(n) }
    { }

    static detail::box_type<symbol> create(detail::string_type const &n);
    static detail::box_type<symbol> create(detail::string_type const &ns, detail::string_type const &name);

    detail::boolean_type equal(object const &) const override;
    detail::string_type to_string() const override;
    detail::integer_type to_hash() const override;

    symbol const* as_symbol() const override;

    bool operator ==(symbol const &rhs) const;

    detail::string_type ns;
    detail::string_type name;
  };
  using symbol_ptr = detail::box_type<obj::symbol>;
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
    bool operator()(jank::runtime::obj::symbol_ptr const &lhs, jank::runtime::obj::symbol_ptr const &rhs) const noexcept
    {
      if(!lhs)
      { return !rhs; }
      else if(!rhs)
      { return !lhs; }
      return lhs->equal(*rhs);
    }
  };
}
