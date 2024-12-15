#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using persistent_array_map_ptr = native_box<struct persistent_array_map>;
  using symbol_ptr = native_box<struct symbol>;
  using keyword_ptr = native_box<struct keyword>;

  /* The correct way to create a keyword for normal use is through interning via the RT context. */
  struct keyword : gc
  {
    static constexpr object_type obj_type{ object_type::keyword };
    static constexpr native_bool pointer_free{ false };
    /* Clojure uses this. No idea. https://github.com/clojure/clojure/blob/master/src/jvm/clojure/lang/Keyword.java */
    static constexpr size_t hash_magic{ 0x9e3779b9 };

    keyword() = default;
    keyword(keyword &&) noexcept = default;
    keyword(keyword const &) = default;
    keyword(runtime::detail::must_be_interned, native_persistent_string_view const &s);
    keyword(runtime::detail::must_be_interned,
            native_persistent_string_view const &ns,
            native_persistent_string_view const &n);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::comparable */
    native_integer compare(object const &) const;

    /* behavior::comparable extended */
    native_integer compare(keyword const &) const;

    /* behavior::nameable */
    native_persistent_string const &get_name() const;
    native_persistent_string const &get_namespace() const;

    /* behavior::callable */
    object_ptr call(object_ptr);
    object_ptr call(object_ptr, object_ptr);

    native_bool operator==(keyword const &rhs) const;

    object base{ obj_type };
    symbol_ptr sym;
  };
}

namespace std
{
  template <>
  struct hash<jank::runtime::obj::keyword_ptr>
  {
    size_t operator()(jank::runtime::obj::keyword_ptr const o) const noexcept
    {
      return o->to_hash();
    }
  };

  template <>
  struct hash<jank::runtime::obj::keyword>
  {
    size_t operator()(jank::runtime::obj::keyword const &o) const noexcept
    {
      static auto hasher(std::hash<jank::runtime::obj::keyword_ptr>{});
      return hasher(const_cast<jank::runtime::obj::keyword *>(&o));
    }
  };

  template <>
  struct equal_to<jank::runtime::obj::keyword_ptr>
  {
    bool operator()(jank::runtime::obj::keyword_ptr const &lhs,
                    jank::runtime::obj::keyword_ptr const &rhs) const noexcept
    {
      return lhs == rhs;
    }
  };
}
