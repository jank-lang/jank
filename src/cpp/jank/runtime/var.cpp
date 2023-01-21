#include <fmt/compile.h>

#include <jank/runtime/var.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/hash.hpp>

namespace jank::runtime
{
  /* TODO: If ns already has var, don't make a new one. */
  var::var(ns_ptr const &n, obj::symbol_ptr const &s)
    : n{ n }, name{ s }
  { }
  var::var(ns_ptr const &n, obj::symbol_ptr const &s, object_ptr o)
    : n{ n }, name{ s }, root{ o }
  { }

  native_bool var::equal(object const &o) const
  {
    auto const *v(o.as_var());
    if(!v)
    { return false; }

    return n == v->n && name == v->name;
  }

  void to_string_impl(obj::symbol_ptr const &name, fmt::memory_buffer &buff)
  { format_to(std::back_inserter(buff), FMT_COMPILE("#'{}/{}"), name->ns, name->name); }
  void var::to_string(fmt::memory_buffer &buff) const
  { to_string_impl(name, buff); }
  native_string var::to_string() const
  /* TODO: Maybe cache this. */
  {
    fmt::memory_buffer buff;
    to_string_impl(name, buff);
    return native_string{ buff.data(), buff.size() };
  }
  native_integer var::to_hash() const
  /* TODO: Cache this. */
  { return detail::hash_combine(n->name->to_hash(), name->to_hash()); }

  var const* var::as_var() const
  { return this; }

  object_ptr var::with_meta(object_ptr m) const
  {
    validate_meta(m);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    const_cast<var*>(this)->meta = m;
    return const_cast<var*>(this);
  }

  behavior::metadatable const* var::as_metadatable() const
  { return this; }

  object_ptr var::get_root() const
  { return *root.rlock(); }

  var_ptr var::set_root(object_ptr r)
  {
    *root.wlock() = r;
    return this;
  }
}
