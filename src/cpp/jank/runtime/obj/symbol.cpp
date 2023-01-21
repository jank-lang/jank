#include <iostream>
#include <sstream>

#include <fmt/compile.h>

#include <jank/runtime/obj/symbol.hpp>

namespace jank::runtime::obj
{
  template <typename S>
  void separate(symbol &sym, S &&s)
  {
    auto const found(s.find('/'));
    if(found != native_string::npos)
    {
      sym.ns = s.substr(0, found);
      sym.name = s.substr(found + 1);
    }
    else
    { sym.name = std::forward<S>(s); }
  }

  symbol::symbol(native_string const &d)
  { separate(*this, d); }
  symbol::symbol(native_string &&d)
  { separate(*this, std::move(d)); }

  symbol::symbol(native_string const &ns, native_string const &n)
    : ns{ ns }, name{ n }
  { }
  symbol::symbol(native_string &&ns, native_string &&n)
    : ns{ std::move(ns) }, name{ std::move(n) }
  { }

  native_bool symbol::equal(object const &o) const
  {
    auto const *s(o.as_symbol());
    if(!s)
    { return false; }

    return ns == s->ns && name == s->name;
  }

  void to_string_impl
  (
    native_string const &ns,
    native_string const &name,
    fmt::memory_buffer &buff
  )
  {
    if(!ns.empty())
    { format_to(std::back_inserter(buff), FMT_COMPILE("{}/{}"), ns, name); }
    else
    { format_to(std::back_inserter(buff), FMT_COMPILE("{}"), name); }
  }
  void symbol::to_string(fmt::memory_buffer &buff) const
  { to_string_impl(ns, name, buff); }
  native_string symbol::to_string() const
  {
    fmt::memory_buffer buff;
    to_string_impl(ns, name, buff);
    return native_string{ buff.data(), buff.size() };
  }
  native_integer symbol::to_hash() const
  /* TODO: Cache this. */
  { return runtime::detail::hash_combine(ns.to_hash(), name.to_hash()); }

  symbol const* symbol::as_symbol() const
  { return this; }

  object_ptr symbol::with_meta(object_ptr m) const
  {
    validate_meta(m);
    auto ret(jank::make_box<symbol>(ns, name));
    ret->meta = m;
    return ret;
  }

  behavior::metadatable const* symbol::as_metadatable() const
  { return this; }

  bool symbol::operator ==(symbol const &rhs) const
  { return ns == rhs.ns && name == rhs.name; }
}
