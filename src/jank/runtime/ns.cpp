#include <memory>

#include <jank/runtime/ns.hpp>
#include <jank/runtime/type/fn.hpp>
#include <jank/runtime/type/string.hpp>

namespace jank::runtime
{
  ns_ptr ns::create(type::symbol_ptr const &n, context const &c)
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
  { return "ns(" + name->to_string() + ")"; }
  runtime::detail::integer_type ns::to_hash() const
  /* TODO: Cache this. */
  { return name->to_hash(); }

  ns const* ns::as_ns() const
  { return this; }

  bool ns::operator ==(ns const &rhs) const
  { return name == rhs.name; }
}
