#include <iostream>
#include <sstream>

#include <jank/runtime/obj/symbol.hpp>

namespace jank::runtime::obj
{
  template <typename S>
  void separate(symbol &sym, S &&s)
  {
    auto const found(s.data->find('/'));
    if(found != std::string::npos)
    {
      sym.ns = s.data->substr(0, found);
      sym.name = s.data->substr(found + 1);
    }
    else
    { sym.name = std::forward<S>(s); }
  }

  symbol::symbol(runtime::detail::string_type const &d)
  { separate(*this, d); }
  symbol::symbol(runtime::detail::string_type &&d)
  { separate(*this, std::move(d)); }

  symbol::symbol(runtime::detail::string_type const &ns, runtime::detail::string_type const &n)
    : ns{ ns }, name{ n }
  { }
  symbol::symbol(runtime::detail::string_type &&ns, runtime::detail::string_type &&n)
    : ns{ std::move(ns) }, name{ std::move(n) }
  { }

  runtime::detail::box_type<symbol> symbol::create(runtime::detail::string_type const &n)
  { return make_box<symbol>(n); }
  runtime::detail::box_type<symbol> symbol::create(runtime::detail::string_type &&n)
  { return make_box<symbol>(std::move(n)); }

  runtime::detail::box_type<symbol> symbol::create(runtime::detail::string_type const &ns, runtime::detail::string_type const &name)
  { return make_box<symbol>(ns, name); }
  runtime::detail::box_type<symbol> symbol::create(runtime::detail::string_type &&ns, runtime::detail::string_type &&name)
  { return make_box<symbol>(std::move(ns), std::move(name)); }

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
  { return runtime::detail::hash_combine(ns.to_hash(), name.to_hash()); }

  symbol const* symbol::as_symbol() const
  { return this; }

  bool symbol::operator ==(symbol const &rhs) const
  { return ns == rhs.ns && name == rhs.name; }
}
