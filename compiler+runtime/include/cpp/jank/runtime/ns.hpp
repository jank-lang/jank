#pragma once

#include <folly/Synchronized.h>

#include <jank/runtime/var.hpp>

namespace jank::runtime
{
  struct context;

  namespace obj
  {
    using symbol_ptr = native_box<struct symbol>;
  }

  using ns_ptr = native_box<struct ns>;

  struct ns : gc
  {
    static constexpr object_type obj_type{ object_type::ns };
    static constexpr native_bool pointer_free{ false };

    ns() = delete;
    ns(obj::symbol_ptr const &name, context &c);

    var_ptr intern_var(native_persistent_string_view const &);
    var_ptr intern_var(obj::symbol_ptr const &);
    option<var_ptr> find_var(obj::symbol_ptr const &);
    result<void, native_persistent_string> unmap(obj::symbol_ptr const &sym);

    result<void, native_persistent_string> add_alias(obj::symbol_ptr const &sym, ns_ptr const &ns);
    void remove_alias(obj::symbol_ptr const &sym);
    option<ns_ptr> find_alias(obj::symbol_ptr const &sym) const;

    result<void, native_persistent_string> refer(obj::symbol_ptr const sym, var_ptr const var);

    obj::persistent_hash_map_ptr get_mappings() const;

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    native_persistent_string to_code_string() const;
    void to_string(util::string_builder &buff) const;
    native_hash to_hash() const;

    native_bool operator==(ns const &rhs) const;

    ns_ptr clone(context &rt_ctx) const;

    object base{ obj_type };
    obj::symbol_ptr name{};
    /* TODO: Benchmark the use of atomics here. That's what Clojure uses. */
    folly::Synchronized<obj::persistent_hash_map_ptr> vars;
    folly::Synchronized<obj::persistent_hash_map_ptr> aliases;

    std::atomic_uint64_t symbol_counter{};
    context &rt_ctx;
  };
}
