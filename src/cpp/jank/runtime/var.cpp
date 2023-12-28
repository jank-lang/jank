#include <fmt/compile.h>

#include <jank/runtime/var.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime
{
  /* TODO: If ns already has var, don't make a new one. */
  var::static_object(ns_ptr const &n, obj::symbol_ptr const &s)
    : n{ n }, name{ s }
  { }
  var::static_object(ns_ptr const &n, obj::symbol_ptr const &s, object_ptr o)
    : n{ n }, name{ s }, root{ o }
  { }

  native_bool var::equal(object const &o) const
  {
    if(o.type != object_type::var)
    { return false; }

    auto const v(expect_object<var>(&o));
    return n == v->n && name == v->name;
  }

  native_bool var::equal(var const &v) const
  { return n == v.n && name == v.name; }

  void to_string_impl(ns_ptr const n, obj::symbol_ptr const &name, fmt::memory_buffer &buff)
  { format_to(std::back_inserter(buff), FMT_COMPILE("#'{}/{}"), n->name->name, name->name); }
  void var::to_string(fmt::memory_buffer &buff) const
  { to_string_impl(n, name, buff); }
  native_persistent_string var::to_string() const
  /* TODO: Maybe cache this. */
  {
    fmt::memory_buffer buff;
    to_string_impl(n, name, buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }
  native_integer var::to_hash() const
  /* TODO: Cache this. */
  { return detail::hash_combine(n->name->to_hash(), name->to_hash()); }

  object_ptr var::with_meta(object_ptr const m)
  {
    meta = behavior::detail::validate_meta(m);
    return this;
  }

  object_ptr var::get_root() const
  {
    profile::timer timer{ "var get_root" };
    return *root.rlock();
  }

  var_ptr var::set_root(object_ptr r)
  {
    profile::timer timer{ "var set_root" };
    *root.wlock() = r;
    return this;
  }

  var_ptr var::clone() const
  { return make_box<var>(n, name, get_root()); }
}
