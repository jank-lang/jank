#pragma once

#include <functional>

#include <folly/Synchronized.h>

#include <jank/result.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/symbol.hpp>

namespace jank::runtime
{
  using ns_ptr = native_box<struct ns>;
  using var_ptr = native_box<struct var>;
  using var_thread_binding_ptr = native_box<struct var_thread_binding>;
  using var_unbound_root_ptr = native_box<struct var_unbound_root>;

  namespace obj
  {
    using persistent_hash_map_ptr = native_box<struct persistent_hash_map>;
  }

  struct var : gc
  {
    static constexpr object_type obj_type{ object_type::var };
    static constexpr native_bool pointer_free{ false };

    var() = delete;
    var(ns_ptr const &n, obj::symbol_ptr const &name);
    var(ns_ptr const &n, obj::symbol_ptr const &name, object_ptr root);
    var(ns_ptr const &n,
        obj::symbol_ptr const &name,
        object_ptr const root,
        native_bool dynamic,
        native_bool thread_bound);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    native_persistent_string to_code_string() const;
    void to_string(util::string_builder &buff) const;
    native_hash to_hash() const;

    /* behavior::object_like extended */
    native_bool equal(var const &) const;

    /* behavior::metadatable */
    var_ptr with_meta(object_ptr m);

    native_bool is_bound() const;
    object_ptr get_root() const;
    /* Binding a root changes it for all threads. */
    var_ptr bind_root(object_ptr r);
    object_ptr alter_root(object_ptr f, object_ptr args);
    /* Setting a var does not change its root, it only affects the current thread
     * binding. If there is no thread binding, a var cannot be set. */
    string_result<void> set(object_ptr r) const;

    var_ptr set_dynamic(native_bool dyn);

    var_thread_binding_ptr get_thread_binding() const;

    /* behavior::derefable */
    object_ptr deref() const;

    native_bool operator==(var const &rhs) const;

    var_ptr clone() const;

    object base{ obj_type };
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

  struct var_thread_binding : gc
  {
    static constexpr object_type obj_type{ object_type::var_thread_binding };
    static constexpr native_bool pointer_free{ false };

    var_thread_binding(object_ptr value, std::thread::id id);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    object base{ obj_type };
    object_ptr value{};
    std::thread::id thread_id;
  };

  struct thread_binding_frame
  {
    obj::persistent_hash_map_ptr bindings{};
  };

  struct var_unbound_root : gc
  {
    static constexpr object_type obj_type{ object_type::var_unbound_root };
    static constexpr native_bool pointer_free{ true };

    var_unbound_root(var_ptr var);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    object base{ obj_type };
    var_ptr var{};
  };
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
        return false;
      }
      return lhs->equal(*rhs);
    }
  };
}
