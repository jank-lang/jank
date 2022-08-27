#include <iostream>
#include <sstream>

#include <jank/runtime/obj/keyword.hpp>

namespace jank::runtime::obj
{
  keyword::keyword(symbol const &s, bool const resolved)
    : sym{ s }, resolved{ resolved }
  { }
  keyword::keyword(symbol &&s, bool const resolved)
    : sym{ std::move(s) }, resolved{ resolved }
  { }

  keyword_ptr keyword::create(symbol const &s, bool const resolved)
  { return make_box<keyword>(s, resolved); }
  keyword_ptr keyword::create(symbol &&s, bool const resolved)
  { return make_box<keyword>(std::move(s), resolved); }

  runtime::detail::boolean_type keyword::equal(object const &o) const
  {
    auto const *s(o.as_keyword());
    if(!s)
    { return false; }

    return resolved == s->resolved && sym == s->sym;
  }
  runtime::detail::string_type keyword::to_string() const
  { return "keyword(" + sym.to_string() + ")"; }
  runtime::detail::integer_type keyword::to_hash() const
  /* TODO: Cache this. */
  { return detail::hash_combine(sym.to_hash(), hash_magic); }

  keyword const* keyword::as_keyword() const
  { return this; }

  bool keyword::operator ==(keyword const &rhs) const
  { return resolved == rhs.resolved && sym == rhs.sym; }
}
