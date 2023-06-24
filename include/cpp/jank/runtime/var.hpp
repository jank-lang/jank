#pragma once

#include <functional>
#include <mutex>

#include <libguarded/shared_guarded.hpp>

#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime
{
  struct ns;
  using ns_ptr = native_box<ns>;

  struct var : object, behavior::metadatable
  {
    using var_ptr = native_box<var>;

    static constexpr bool pointer_free{ false };

    var(var const&) = delete;
    var(var &&) noexcept = default;
    var(ns_ptr const &n, obj::symbol_ptr const &s);
    var(ns_ptr const &n, obj::symbol_ptr const &s, object_ptr o);

    native_bool equal(object const &) const final;
    native_string to_string() const final;
    void to_string(fmt::memory_buffer &buff) const final;
    native_integer to_hash() const final;

    var const* as_var() const final;

    object_ptr with_meta(object_ptr m) const final;
    behavior::metadatable const* as_metadatable() const final;

    bool operator ==(var const &rhs) const;

    object_ptr get_root() const;
    native_box<var> set_root(object_ptr r);

    var_ptr clone() const;

    ns_ptr n;
    /* TODO: Make sure this gets fully qualified. */
    obj::symbol_ptr name;

  private:
    libguarded::shared_guarded<object_ptr> root;
  };
  using var_ptr = native_box<var>;
}
