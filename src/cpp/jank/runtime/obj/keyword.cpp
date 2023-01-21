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
  { return jank::make_box<keyword>(s, resolved); }
  keyword_ptr keyword::create(symbol &&s, bool const resolved)
  { return jank::make_box<keyword>(std::move(s), resolved); }

  native_bool keyword::equal(object const &o) const
  {
    auto const *s(o.as_keyword());
    if(!s)
    { return false; }

    return resolved == s->resolved && sym == s->sym;
  }


  void to_string_impl(symbol const &sym, fmt::memory_buffer &buff)
  {
    std::back_inserter(buff) = ':';
    sym.to_string(buff);
  }
  void keyword::to_string(fmt::memory_buffer &buff) const
  { return to_string_impl(sym, buff); }
  native_string keyword::to_string() const
  {
    fmt::memory_buffer buff;
    to_string_impl(sym, buff);
    return native_string{ buff.data(), buff.size() };
  }
  native_integer keyword::to_hash() const
  /* TODO: Cache this. */
  { return runtime::detail::hash_combine(sym.to_hash(), hash_magic); }

  keyword const* keyword::as_keyword() const
  { return this; }

  bool keyword::operator ==(keyword const &rhs) const
  { return resolved == rhs.resolved && sym == rhs.sym; }
}
