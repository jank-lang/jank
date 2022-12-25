#include <memory>

#include <jank/runtime/ns.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/string.hpp>

namespace jank::runtime
{
  ns_ptr ns::create(obj::symbol_ptr const &n, context const &c)
  { return make_box<ns>(n, c); }

  runtime::detail::boolean_type ns::equal(object const &o) const
  {
    auto const *v(o.as_ns());
    if(!v)
    { return false; }

    return name == v->name;
  }
  runtime::detail::string_type ns::to_string() const
  /* TODO: Maybe cache this. */
  { return name->to_string(); }
  void ns::to_string(fmt::memory_buffer &buff) const
  { name->to_string(buff); }
  runtime::detail::integer_type ns::to_hash() const
  /* TODO: Cache this. */
  { return name->to_hash(); }

  ns const* ns::as_ns() const
  { return this; }

  bool ns::operator ==(ns const &rhs) const
  { return name == rhs.name; }
}
