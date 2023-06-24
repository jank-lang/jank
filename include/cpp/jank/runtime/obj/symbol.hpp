#pragma once

#include <functional>

#include <jank/runtime/hash.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::obj
{
  struct symbol : object, behavior::metadatable
  {
    static constexpr bool pointer_free{ true };

    symbol() = default;
    symbol(symbol &&) = default;
    symbol(symbol const &) = default;
    symbol(native_string const &d);
    symbol(native_string &&d);
    symbol(native_string const &ns, native_string const &n);
    symbol(native_string &&ns, native_string &&n);

    native_bool equal(object const &) const final;
    native_string to_string() const final;
    void to_string(fmt::memory_buffer &buff) const final;
    native_integer to_hash() const final;

    symbol const* as_symbol() const final;

    object_ptr with_meta(object_ptr m) const final;
    behavior::metadatable const* as_metadatable() const final;

    bool operator ==(symbol const &rhs) const;

    symbol& operator =(symbol const &) = default;
    symbol& operator =(symbol &&) = default;

    native_string ns;
    native_string name;
  };
  using symbol_ptr = obj::symbol*;
}

namespace std
{
  template <>
  struct hash<jank::runtime::obj::symbol>
  {
    size_t operator()(jank::runtime::obj::symbol const &o) const noexcept
    {
      static auto hasher(std::hash<jank::native_string>{});
      return static_cast<size_t>(jank::runtime::detail::hash_combine(hasher(o.ns), hasher(o.name)));
    }
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
