#pragma once

#include <functional>

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
    static constexpr native_bool pointer_free{ false };

    static_object() = delete;
    static_object(ns_ptr const &n, obj::symbol_ptr const &name);
    static_object(ns_ptr const &n, obj::symbol_ptr const &name, object_ptr root);
    static_object(ns_ptr const &n,
                  obj::symbol_ptr const &name,
                  object_ptr const root,
                  native_bool dynamic,
                  native_bool thread_bound);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    /* behavior::object_like extended */
    native_bool equal(static_object const &) const;

    /* behavior::metadatable */
    native_box<static_object> with_meta(object_ptr m);

    native_bool is_bound() const;
    object_ptr get_root() const;
    /* Binding a root changes it for all threads. */
    native_box<static_object> bind_root(object_ptr r);
    object_ptr alter_root(object_ptr f, object_ptr args);
    /* Setting a var does not change its root, it only affects the current thread
     * binding. If there is no thread binding, a var cannot be set. */
    string_result<void> set(object_ptr r) const;

    native_box<static_object> set_dynamic(native_bool dyn);

    native_box<static_object<object_type::var_thread_binding>> get_thread_binding() const;

    /* behavior::derefable */
    object_ptr deref() const;

    native_bool operator==(static_object const &rhs) const;

    native_box<static_object> clone() const;

    object base{ object_type::var };
    ns_ptr n{};
    /* Unqualified. */
    obj::symbol_ptr name{};
    option<object_ptr> meta;
    mutable native_hash hash{};

  private:
    folly::Synchronized<object_ptr> root;

  public:
    std::atomic_bool dynamic{ false };
    std::atomic_bool thread_bound{ false };
  };

  using var = static_object<object_type::var>;
  using var_ptr = native_box<var>;

  template <>
  struct static_object<object_type::var_thread_binding> : gc
  {
    static constexpr native_bool pointer_free{ false };

    static_object(object_ptr value, std::thread::id id);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    object base{ object_type::var_thread_binding };
    object_ptr value{};
    std::thread::id thread_id;
  };

  using var_thread_binding = static_object<object_type::var_thread_binding>;
  using var_thread_binding_ptr = native_box<var_thread_binding>;

  struct thread_binding_frame
  {
    obj::persistent_hash_map_ptr bindings{};
  };

  template <>
  struct static_object<object_type::var_unbound_root> : gc
  {
    static constexpr native_bool pointer_free{ true };

    static_object(var_ptr var);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    object base{ object_type::var_unbound_root };
    var_ptr var{};
  };

  using var_unbound_root = static_object<object_type::var_unbound_root>;
  using var_unbound_root_ptr = native_box<var_unbound_root>;
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
    bool
    operator()(jank::runtime::var_ptr const &lhs, jank::runtime::var_ptr const &rhs) const noexcept
    {
      if(!lhs)
      {
        return !rhs;
      }
      else if(!rhs)
      {
        return !lhs;
      }
      return lhs->equal(*rhs);
    }
  };
}
