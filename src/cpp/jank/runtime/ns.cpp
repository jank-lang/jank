#include <memory>

#include <jank/runtime/ns.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/string.hpp>

namespace jank::runtime
{
  ns_ptr ns::create(obj::symbol_ptr const &n, context const &c)
  { return jank::make_box<ns>(n, c); }

  native_bool ns::equal(object const &o) const
  {
    auto const *v(o.as_ns());
    if(!v)
    { return false; }

    return name == v->name;
  }
  native_string ns::to_string() const
  /* TODO: Maybe cache this. */
  { return name->to_string(); }
  void ns::to_string(fmt::memory_buffer &buff) const
  { name->to_string(buff); }
  native_integer ns::to_hash() const
  /* TODO: Cache this. */
  { return name->to_hash(); }

  ns const* ns::as_ns() const
  { return this; }

  bool ns::operator ==(ns const &rhs) const
  { return name == rhs.name; }

  ns_ptr ns::clone() const
  {
    auto ret(jank::make_box<ns>(name, rt_ctx));
    auto const ret_locked_vars(ret->vars.wlock());
    auto const locked_vars(vars.rlock());
    for(auto const & var : *locked_vars)
    { ret_locked_vars->insert({var.first, var.second->clone()}); }
    return ret;
  }
}
