#pragma once

#include <functional>
#include <unordered_map>
#include <mutex>

#include <folly/Synchronized.h>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/var.hpp>

namespace jank::runtime
{
  struct context;

  template <>
  struct static_object<object_type::ns> : gc
  {
    static constexpr bool pointer_free{ false };

    static_object() = delete;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(obj::symbol_ptr const &name, context const &c);

    result<void, native_string> add_alias(obj::symbol_ptr const &sym, native_box<static_object> const &ns);
    option<ns_ptr> find_alias(obj::symbol_ptr const &sym) const;

    result<void, native_string> refer(obj::symbol_ptr const sym, var_ptr const var);

    obj::persistent_hash_map_ptr get_mappings() const;

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_integer to_hash() const;

    bool operator ==(static_object const &rhs) const;

    native_box<static_object> clone() const;

    object base{ object_type::ns };
    obj::symbol_ptr name{};
    /* TODO: Clojure doesn't qualify these symbol keys, but we do. That changes the output of
     * fns like `ns-map`. */
    /* TODO: Both of these should be atomic boxes to hash maps. */
    folly::Synchronized<native_unordered_map<obj::symbol_ptr, var_ptr>> vars;
    folly::Synchronized<native_unordered_map<obj::symbol_ptr, ns_ptr>> aliases;
    context const &rt_ctx;
  };

  using ns = static_object<object_type::ns>;
  using ns_ptr = native_box<ns>;
}
