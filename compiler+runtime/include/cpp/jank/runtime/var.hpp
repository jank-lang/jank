#pragma once

#include <functional>

#include <folly/Synchronized.h>

#include <jtl/result.hpp>

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>

namespace jank::runtime
{
  using ns_ref = oref<struct ns>;
  using var_ref = oref<struct var>;
  using var_thread_binding_ref = oref<struct var_thread_binding>;
  using var_unbound_root_ref = oref<struct var_unbound_root>;

  struct var : object
  {
    static constexpr object_type obj_type{ object_type::var };
    static constexpr object_behavior obj_behaviors{ object_behavior::call };
    static constexpr bool pointer_free{ false };

    var() = delete;
    var(ns_ref const n, obj::symbol_ref const name);
    var(ns_ref const n, obj::symbol_ref const name, object_ref const root);
    var(ns_ref const n,
        obj::symbol_ref const name,
        object_ref const root,
        bool dynamic,
        bool thread_bound);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    jtl::immutable_string to_code_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    uhash to_hash() const override;

    /* behavior::object_like extended */
    bool equal(var const &) const;

    bool is_bound() const;
    object_ref get_root() const;
    /* Binding a root changes it for all threads. */
    var_ref bind_root(object_ref const r);
    object_ref alter_root(object_ref const f, object_ref const args);
    /* Setting a var does not change its root, it only affects the current thread
     * binding. If there is no thread binding, a var cannot be set. */
    jtl::string_result<void> set(object_ref const r) const;

    var_ref set_dynamic(bool dyn);

    obj::symbol_ref to_qualified_symbol() const;
    var_thread_binding_ref get_thread_binding() const;

    /* behavior::callable */
    using object::call;
    object_ref call(object_ref const) const override;
    callable_arity_flags get_arity_flags() const override;

    /* behavior::derefable */
    object_ref deref() const;

    /* behavior::metadatable */
    var_ref with_meta(object_ref m);
    object_ref get_meta() const;
    void set_meta(object_ref const m);

    bool operator==(var const &rhs) const;

    var_ref clone() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    ns_ref n;
    /* Unqualified. */
    obj::symbol_ref name{};

    /*** XXX: Everything here is thread-safe. ***/
  private:
    folly::Synchronized<object_ref> root;
    folly::Synchronized<object_ref> meta;

  public:
    std::atomic_bool dynamic{ false };
    std::atomic_bool thread_bound{ false };
  };

  struct var_thread_binding : object
  {
    static constexpr object_type obj_type{ object_type::var_thread_binding };
    static constexpr object_behavior obj_behaviors{ object_behavior::none };
    static constexpr bool pointer_free{ false };

    var_thread_binding(object_ref const value, std::thread::id id);

    /* behavior::object_like */
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /*** XXX: Everything here is immutable after initialization. ***/
    object_ref value{};
    std::thread::id thread_id;
  };

  struct thread_binding_frame
  {
    obj::persistent_hash_map_ref bindings{};
  };

  struct var_unbound_root : object
  {
    static constexpr object_type obj_type{ object_type::var_unbound_root };
    static constexpr object_behavior obj_behaviors{ object_behavior::none };
    static constexpr bool pointer_free{ true };

    var_unbound_root(var_ref const var);

    /* behavior::object_like */
    using object::to_string;
    void to_string(jtl::string_builder &buff) const override;

    /*** XXX: Everything here is immutable after initialization. ***/
    var_ref var;
  };
}

namespace std
{
  template <>
  struct hash<jank::runtime::var>
  {
    size_t operator()(jank::runtime::var const &o) const noexcept;
  };

  template <>
  struct hash<jank::runtime::var_ref>
  {
    size_t operator()(jank::runtime::var_ref const o) const noexcept;
  };

  template <>
  struct equal_to<jank::runtime::var_ref>
  {
    bool
    operator()(jank::runtime::var_ref const lhs, jank::runtime::var_ref const rhs) const noexcept;
  };
}
