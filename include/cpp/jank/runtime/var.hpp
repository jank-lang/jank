#pragma once

#include <functional>
#include <mutex>

#include <libguarded/shared_guarded.hpp>

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime
{
  struct ns;
  using ns_ptr = detail::box_type<ns>;

  struct var : object, pool_item_base<var>, behavior::metadatable
  {
    var(var const&) = delete;
    var(var &&) noexcept = default;
    var(ns_ptr const &n, obj::symbol_ptr const &s);
    var(ns_ptr const &n, obj::symbol_ptr const &s, object_ptr const &o);

    static detail::box_type<var> create(ns_ptr const &n, obj::symbol_ptr const &s);
    static detail::box_type<var> create(ns_ptr const &n, obj::symbol_ptr const &s, object_ptr const &root);

    detail::boolean_type equal(object const &) const override;
    detail::string_type to_string() const override;
    void to_string(fmt::memory_buffer &buff) const override;
    detail::integer_type to_hash() const override;

    var const* as_var() const override;

    object_ptr with_meta(object_ptr const &m) const override;
    behavior::metadatable const* as_metadatable() const override;

    bool operator ==(var const &rhs) const;

    object_ptr get_root() const;
    detail::box_type<var> set_root(object_ptr const &r);

    ns_ptr n;
    /* TODO: Make sure this gets fully qualified. */
    obj::symbol_ptr name;

  private:
    libguarded::shared_guarded<object_ptr> root;
  };
  using var_ptr = detail::box_type<var>;
}
