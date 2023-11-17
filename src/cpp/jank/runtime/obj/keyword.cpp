#include <iostream>
#include <sstream>

#include <jank/runtime/obj/keyword.hpp>

namespace jank::runtime
{
  obj::keyword::static_object(obj::symbol const &s)
    : sym{ s }
  { }
  obj::keyword::static_object(obj::symbol &&s)
    : sym{ std::move(s) }
  { }

  /* Keywords are interned, so we can always count on identity equality. */
  native_bool obj::keyword::equal(object const &o) const
  { return &base == &o; }

  void to_string_impl(obj::symbol const &sym, fmt::memory_buffer &buff)
  {
    std::back_inserter(buff) = ':';
    sym.to_string(buff);
  }
  void obj::keyword::to_string(fmt::memory_buffer &buff) const
  { return to_string_impl(sym, buff); }
  native_string obj::keyword::to_string() const
  {
    fmt::memory_buffer buff;
    to_string_impl(sym, buff);
    return native_string{ buff.data(), buff.size() };
  }
  native_integer obj::keyword::to_hash() const
  { return reinterpret_cast<native_integer>(this); }

  object_ptr obj::keyword::with_meta(object_ptr m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<obj::keyword>(sym));
    ret->meta = meta;
    return ret;
  }

  bool obj::keyword::operator ==(obj::keyword const &rhs) const
  { return sym == rhs.sym; }
}
