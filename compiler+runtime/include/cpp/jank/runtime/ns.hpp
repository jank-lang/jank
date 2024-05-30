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
    static constexpr native_bool pointer_free{ false };

    static_object() = delete;
    static_object(obj::symbol_ptr const &name, context &c);

    var_ptr intern_var(native_persistent_string_view const &);
    var_ptr intern_var(obj::symbol_ptr const &);
    option<var_ptr> find_var(obj::symbol_ptr const &);

    result<void, native_persistent_string>
    add_alias(obj::symbol_ptr const &sym, native_box<static_object> const &ns);
    option<ns_ptr> find_alias(obj::symbol_ptr const &sym) const;

    result<void, native_persistent_string> refer(obj::symbol_ptr const sym, var_ptr const var);

    obj::persistent_hash_map_ptr get_mappings() const;

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    native_bool operator==(static_object const &rhs) const;

    native_box<static_object> clone(context &rt_ctx) const;

    object base{ object_type::ns };
    obj::symbol_ptr name{};
    /* TODO: Benchmark the use of atomics here. That's what Clojure uses. */
    folly::Synchronized<obj::persistent_hash_map_ptr> vars;
    folly::Synchronized<obj::persistent_hash_map_ptr> aliases;
    context &rt_ctx;
  };

  using ns = static_object<object_type::ns>;
  using ns_ptr = native_box<ns>;
}
