#pragma once

#include <functional>
#include <mutex>

#include <folly/Synchronized.h>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime
{
  using ns = static_object<object_type::ns>;
  using ns_ptr = native_box<ns>;

  template <>
  struct static_object<object_type::var> : gc
  {
    static constexpr bool pointer_free{ false };

    static_object() = delete;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(ns_ptr const &n, obj::symbol_ptr const &s);
    static_object(ns_ptr const &n, obj::symbol_ptr const &s, object_ptr o);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_integer to_hash() const;

    /* behavior::objectable extended */
    native_bool equal(static_object const &) const;

    /* behavior::metadatable */
    object_ptr with_meta(object_ptr m);

    object_ptr get_root() const;
    native_box<static_object> set_root(object_ptr r);

    bool operator ==(static_object const &rhs) const;

    native_box<static_object> clone() const;

    object base{ object_type::var };
    ns_ptr n;
    /* TODO: Make sure this gets fully qualified. */
    obj::symbol_ptr name;
    option<object_ptr> meta;

  private:
    folly::Synchronized<object_ptr> root;
  };

  using var = static_object<object_type::var>;
  using var_ptr = native_box<var>;
}

namespace std
{
  template <>
  struct hash<jank::runtime::var>
  {
    size_t operator()(jank::runtime::var const &o) const noexcept
    {
      static auto hasher(std::hash<jank::runtime::obj::symbol>{});
      return hasher(*o.name);
    }
  };

  template <>
  struct hash<jank::runtime::var_ptr>
  {
    size_t operator()(jank::runtime::var_ptr const &o) const noexcept
    {
      static auto hasher(std::hash<jank::runtime::obj::symbol>{});
      return hasher(*o->name);
    }
  };

  template <>
  struct equal_to<jank::runtime::var_ptr>
  {
    bool operator()
    (
      jank::runtime::var_ptr const &lhs,
      jank::runtime::var_ptr const &rhs
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
