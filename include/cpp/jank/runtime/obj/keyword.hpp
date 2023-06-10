#pragma once

#include <functional>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/hash.hpp>

namespace jank::runtime::obj
{
  /* The correct way to create a keyword for normal use is through interning via the RT context. */
  struct keyword : object
  {
    static constexpr bool pointer_free{ true };

    keyword() = default;
    keyword(keyword &&) = default;
    keyword(keyword const &) = default;
    keyword(symbol const &s, bool const resolved);
    keyword(symbol &&s, bool const resolved);
    ~keyword() = default;

    native_bool equal(object const &) const final;
    native_string to_string() const final;
    void to_string(fmt::memory_buffer &buff) const final;
    native_integer to_hash() const final;

    keyword const* as_keyword() const final;

    bool operator ==(keyword const &rhs) const;

    symbol sym;
    /* Not resolved means this is a :: keyword. If ns is set, when this is true, it's an ns alias.
     * Upon interning, this will be resolved. */
    bool resolved{ true };

    /* Clojure uses this. No idea. https://github.com/clojure/clojure/blob/master/src/jvm/clojure/lang/Keyword.java */
    static constexpr size_t hash_magic{ 0x9e3779b9 };
  };
  using keyword_ptr = obj::keyword*;
}

namespace std
{
  template <>
  struct hash<jank::runtime::obj::keyword>
  {
    size_t operator()(jank::runtime::obj::keyword const &o) const noexcept
    { return static_cast<size_t>(jank::runtime::detail::hash_combine(o.sym.to_hash(), jank::runtime::obj::keyword::hash_magic)); }
  };

  template <>
  struct hash<jank::runtime::obj::keyword_ptr>
  {
    size_t operator()(jank::runtime::obj::keyword_ptr const &o) const noexcept
    {
      static auto hasher(std::hash<jank::runtime::obj::keyword>{});
      return hasher(*o);
    }
  };

  template <>
  struct equal_to<jank::runtime::obj::keyword_ptr>
  {
    bool operator()(jank::runtime::obj::keyword_ptr const &lhs, jank::runtime::obj::keyword_ptr const &rhs) const noexcept
    {
      if(!lhs)
      { return !rhs; }
      else if(!rhs)
      { return !lhs; }
      return lhs->equal(*rhs);
    }
  };
}
