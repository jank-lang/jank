#pragma once

#include <functional>

#include <folly/Synchronized.h>

#include <jtl/result.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/symbol.hpp>

namespace jank::runtime
{
  using ns_ref = oref<struct ns>;
  using var_ref = oref<struct var>;
  using var_thread_binding_ref = oref<struct var_thread_binding>;
  using var_unbound_root_ref = oref<struct var_unbound_root>;

  namespace obj
  {
    using persistent_hash_map_ref = oref<struct persistent_hash_map>;
  }

  struct var : gc
  {
    static constexpr object_type obj_type{ object_type::var };
    static constexpr bool pointer_free{ false };

    var() = delete;
    var(ns_ref const &n, obj::symbol_ref const &name);
    var(ns_ref const &n, obj::symbol_ref const &name, object_ref root);
    var(ns_ref const &n,
        obj::symbol_ref const &name,
        object_ref const root,
        bool dynamic,
        bool thread_bound);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    jtl::immutable_string to_code_string() const;
    void to_string(util::string_builder &buff) const;
    uhash to_hash() const;

    /* behavior::object_like extended */
    bool equal(var const &) const;

    /* behavior::metadatable */
    var_ref with_meta(object_ref m);

    bool is_bound() const;
    object_ref get_root() const;
    /* Binding a root changes it for all threads. */
    var_ref bind_root(object_ref r);
    object_ref alter_root(object_ref f, object_ref args);
    /* Setting a var does not change its root, it only affects the current thread
     * binding. If there is no thread binding, a var cannot be set. */
    jtl::string_result<void> set(object_ref r) const;

    var_ref set_dynamic(bool dyn);

    var_thread_binding_ref get_thread_binding() const;

    /* behavior::derefable */
    object_ref deref() const;

    bool operator==(var const &rhs) const;

    var_ref clone() const;

    object base{ obj_type };
    ns_ref n;
    /* Unqualified. */
    obj::symbol_ref name{};
    jtl::option<object_ref> meta;
    mutable uhash hash{};

  private:
    folly::Synchronized<object_ref> root;

  public:
    std::atomic_bool dynamic{ false };
    std::atomic_bool thread_bound{ false };
  };

  struct var_thread_binding : gc
  {
    static constexpr object_type obj_type{ object_type::var_thread_binding };
    static constexpr bool pointer_free{ false };

    var_thread_binding(object_ref value, std::thread::id id);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    object base{ obj_type };
    object_ref value{};
    std::thread::id thread_id;
  };

  struct thread_binding_frame
  {
    obj::persistent_hash_map_ref bindings{};
  };

  struct var_unbound_root : gc
  {
    static constexpr object_type obj_type{ object_type::var_unbound_root };
    static constexpr bool pointer_free{ true };

    var_unbound_root(var_ref var);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    object base{ obj_type };
    var_ref var;
  };
}

/* TODO: Move these to the .cpp */
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
  struct hash<jank::runtime::var_ref>
  {
    size_t operator()(jank::runtime::var_ref const &o) const noexcept
    {
      static auto hasher(std::hash<jank::runtime::obj::symbol>{});
      return hasher(*o->name);
    }
  };

  template <>
  struct equal_to<jank::runtime::var_ref>
  {
    bool
    operator()(jank::runtime::var_ref const &lhs, jank::runtime::var_ref const &rhs) const noexcept
    {
      return lhs->equal(*rhs);
    }
  };
}
