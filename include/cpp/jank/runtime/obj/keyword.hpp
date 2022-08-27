#pragma once

#include <functional>

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/hash.hpp>

namespace jank::runtime::obj
{
  /* The correct way to create a keyword for normal use is through interning via the RT context. */
  struct keyword : object, pool_item_base<keyword>
  {
    keyword() = default;
    keyword(keyword &&) = default;
    keyword(keyword const &) = default;
    keyword(symbol const &s, bool const resolved);
    keyword(symbol &&s, bool const resolved);

    static runtime::detail::box_type<keyword> create(symbol const &s, bool const resolved);
    static runtime::detail::box_type<keyword> create(symbol &&s, bool const resolved);

    runtime::detail::boolean_type equal(object const &) const override;
    runtime::detail::string_type to_string() const override;
    runtime::detail::integer_type to_hash() const override;

    keyword const* as_keyword() const override;

    bool operator ==(keyword const &rhs) const;

    keyword& operator =(keyword const &) = default;
    keyword& operator =(keyword &&) = default;

    symbol sym;
    /* Not resolved means this is a :: keyword. If ns is set, when this is true, it's an ns alias.
     * Upon interning, this will be resolved. */
    bool resolved{ true };

    /* Clojure uses this. No idea. https://github.com/clojure/clojure/blob/master/src/jvm/clojure/lang/Keyword.java */
    static constexpr size_t hash_magic{ 0x9e3779b9 };
  };
  using keyword_ptr = runtime::detail::box_type<obj::keyword>;
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
