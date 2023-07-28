#include <memory>

#include <jank/runtime/ns.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/string.hpp>

namespace jank::runtime
{
  native_bool ns::equal(object const &o) const
  {
    if(o.type != object_type::ns)
    { return false; }

    auto const v(expect_object<ns>(&o));
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
