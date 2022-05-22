#include <iostream>
#include <sstream>

#include <jank/runtime/type/symbol.hpp>

namespace jank::runtime::type
{
  template <typename S>
  void separate(symbol &sym, S &&s)
  {
    auto found(s.data->find('/'));
    if(found != std::string::npos)
    {
      sym.ns = s.data->substr(0, found);
      sym.name = s.data->substr(found + 1);
    }
    sym.name = std::forward<S>(s);
  }

  symbol::symbol(detail::string_type const &d)
  { separate(*this, d); }
  symbol::symbol(detail::string_type &&d)
  { separate(*this, std::move(d)); }

  detail::box_type<symbol> symbol::create(detail::string_type const &n)
  {
    auto found(n.data->find('/'));
    if(found != std::string::npos)
    { return make_box<symbol>(n.data->substr(0, found), n.data->substr(found + 1)); }
    return make_box<symbol>(n);
  }

  detail::box_type<symbol> symbol::create(detail::string_type const &ns, detail::string_type const &name)
  { return make_box<symbol>(ns, name); }

  runtime::detail::boolean_type symbol::equal(object const &o) const
  {
    auto const *s(o.as_symbol());
    if(!s)
    { return false; }

    return ns == s->ns && name == s->name;
  }
  runtime::detail::string_type symbol::to_string() const
  {
    if(ns.length > 0)
    { return "symbol(" + ns + "/" + name + ")"; }
    return "symbol(" + name + ")";
  }
  runtime::detail::integer_type symbol::to_hash() const
  /* TODO: Cache this. */
  { return detail::hash_combine(ns.to_hash(), name.to_hash()); }

  symbol const* symbol::as_symbol() const
  { return this; }

  bool symbol::operator ==(symbol const &rhs) const
  { return ns == rhs.ns && name == rhs.name; }
}
