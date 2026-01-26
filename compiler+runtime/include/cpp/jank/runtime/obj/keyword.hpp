#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  /* The correct way to create a keyword for normal use is through interning via the RT context. */
  struct keyword : object
  {
    static constexpr object_type obj_type{ object_type::keyword };
    static constexpr object_behavior obj_behaviors{ object_behavior::call };
    static constexpr bool pointer_free{ false };
    /* Clojure uses this. No idea. https://github.com/clojure/clojure/blob/master/src/jvm/clojure/lang/Keyword.java */
    static constexpr usize hash_magic{ 0x9e3779b9 };

    keyword();
    keyword(keyword &&) noexcept = default;
    keyword(keyword const &) = default;
    keyword(runtime::detail::must_be_interned, jtl::immutable_string_view const &s);
    keyword(runtime::detail::must_be_interned,
            jtl::immutable_string_view const &ns,
            jtl::immutable_string_view const &n);

    /* behavior::object_like */
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    uhash to_hash() const override;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(keyword const &) const;

    /* behavior::nameable */
    jtl::immutable_string const &get_name() const;
    jtl::immutable_string const &get_namespace() const;

    /* behavior::callable */
    using object::call;
    object_ref call(object_ref const) const override;
    object_ref call(object_ref const, object_ref const) const override;

    bool operator==(keyword const &rhs) const;

    /*** XXX: Everything here is immutable after initialization. ***/
    symbol_ref sym;
  };

  using keyword_ref = oref<keyword>;
}

/* TODO: Move to .cpp */
namespace std
{
  template <>
  struct hash<jank::runtime::obj::keyword_ref>
  {
    size_t operator()(jank::runtime::obj::keyword_ref const o) const
    {
      return o->to_hash();
    }
  };

  template <>
  struct hash<jank::runtime::obj::keyword>
  {
    size_t operator()(jank::runtime::obj::keyword const &o) const
    {
      static auto hasher(std::hash<jank::runtime::obj::keyword_ref>{});
      return hasher(const_cast<jank::runtime::obj::keyword *>(&o));
    }
  };

  template <>
  struct equal_to<jank::runtime::obj::keyword_ref>
  {
    bool operator()(jank::runtime::obj::keyword_ref const lhs,
                    jank::runtime::obj::keyword_ref const rhs) const noexcept
    {
      return lhs.data == rhs.data;
    }
  };
}
