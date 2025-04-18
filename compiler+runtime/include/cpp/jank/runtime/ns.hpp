#pragma once

#include <folly/Synchronized.h>

#include <jank/runtime/var.hpp>

namespace jank::runtime
{
  struct context;

  namespace obj
  {
    using symbol_ref = oref<struct symbol>;
  }

  using ns_ref = oref<struct ns>;

  struct ns : gc
  {
    static constexpr object_type obj_type{ object_type::ns };
    static constexpr bool pointer_free{ false };

    ns() = delete;
    ns(obj::symbol_ref const &name, context &c);

    var_ref intern_var(native_persistent_string_view const &);
    var_ref intern_var(obj::symbol_ref const &);
    var_ref find_var(obj::symbol_ref const &);
    jtl::result<void, jtl::immutable_string> unmap(obj::symbol_ref const &sym);

    jtl::result<void, jtl::immutable_string>
    add_alias(obj::symbol_ref const &sym, ns_ref const &ns);
    void remove_alias(obj::symbol_ref const &sym);
    ns_ref find_alias(obj::symbol_ref const &sym) const;

    jtl::result<void, jtl::immutable_string> refer(obj::symbol_ref const sym, var_ref const var);

    obj::persistent_hash_map_ref get_mappings() const;

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    jtl::immutable_string to_code_string() const;
    void to_string(util::string_builder &buff) const;
    native_hash to_hash() const;

    bool operator==(ns const &rhs) const;

    ns_ref clone(context &rt_ctx) const;

    object base{ obj_type };
    obj::symbol_ref name{};
    /* TODO: Benchmark the use of atomics here. That's what Clojure uses. */
    folly::Synchronized<obj::persistent_hash_map_ref> vars;
    folly::Synchronized<obj::persistent_hash_map_ref> aliases;

    std::atomic_uint64_t symbol_counter{};
    context &rt_ctx;
  };
}
