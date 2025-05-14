#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using persistent_array_map_ref = oref<struct persistent_array_map>;
  using symbol_ref = oref<struct symbol>;
  using keyword_ref = oref<struct keyword>;

  /* The correct way to create a keyword for normal use is through interning via the RT context. */
  struct keyword : gc
  {
    static constexpr object_type obj_type{ object_type::keyword };
    static constexpr bool pointer_free{ false };
    /* Clojure uses this. No idea. https://github.com/clojure/clojure/blob/master/src/jvm/clojure/lang/Keyword.java */
    static constexpr usize hash_magic{ 0x9e3779b9 };

    keyword() = default;
    keyword(keyword &&) noexcept = default;
    keyword(keyword const &) = default;
    keyword(runtime::detail::must_be_interned, native_persistent_string_view const &s);
    keyword(runtime::detail::must_be_interned,
            native_persistent_string_view const &ns,
            native_persistent_string_view const &n);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::comparable */
    i64 compare(object const &) const;

    /* behavior::comparable extended */
    i64 compare(keyword const &) const;

    /* behavior::nameable */
    jtl::immutable_string const &get_name() const;
    jtl::immutable_string const &get_namespace() const;

    /* behavior::callable */
    object_ref call(object_ref);
    object_ref call(object_ref, object_ref);

    bool operator==(keyword const &rhs) const;

    object base{ obj_type };
    symbol_ref sym;
  };
}

/* TODO: Move to .cpp */
namespace std
{
  template <>
  struct hash<jank::runtime::obj::keyword_ref>
  {
    size_t operator()(jank::runtime::obj::keyword_ref const o) const noexcept
    {
      return o->to_hash();
    }
  };

  template <>
  struct hash<jank::runtime::obj::keyword>
  {
    size_t operator()(jank::runtime::obj::keyword const &o) const noexcept
    {
      static auto hasher(std::hash<jank::runtime::obj::keyword_ref>{});
      return hasher(const_cast<jank::runtime::obj::keyword *>(&o));
    }
  };

  template <>
  struct equal_to<jank::runtime::obj::keyword_ref>
  {
    bool operator()(jank::runtime::obj::keyword_ref const &lhs,
                    jank::runtime::obj::keyword_ref const &rhs) const noexcept
    {
      return lhs == rhs;
    }
  };
}
