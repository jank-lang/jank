#include <iostream>
#include <sstream>

#include <jank/runtime/obj/keyword.hpp>

namespace jank::runtime
{
  obj::keyword::static_object(obj::symbol const &s)
    : sym{ s }
  {
  }

  obj::keyword::static_object(obj::symbol &&s)
    : sym{ std::move(s) }
  {
  }

  obj::keyword::static_object(native_persistent_string_view const &ns,
                              native_persistent_string_view const &n)
    : sym{ ns, n }
  {
  }

  /* Keywords are interned, so we can always count on identity equality. */
  native_bool obj::keyword::equal(object const &o) const
  {
    return &base == &o;
  }

  void to_string_impl(obj::symbol const &sym, fmt::memory_buffer &buff)
  {
    std::back_inserter(buff) = ':';
    sym.to_string(buff);
  }

  void obj::keyword::to_string(fmt::memory_buffer &buff) const
  {
    return to_string_impl(sym, buff);
  }

  native_persistent_string obj::keyword::to_string() const
  {
    fmt::memory_buffer buff;
    to_string_impl(sym, buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_hash obj::keyword::to_hash() const
  {
    return sym.to_hash() + 0x9e3779b9;
  }

  object_ptr obj::keyword::with_meta(object_ptr m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<obj::keyword>(sym));
    ret->meta = meta;
    return ret;
  }

  native_persistent_string const &obj::keyword::get_name() const
  {
    return sym.name;
  }

  native_persistent_string const &obj::keyword::get_namespace() const
  {
    return sym.ns;
  }

  object_ptr obj::keyword::call(object_ptr const m) const
  {
    return runtime::get(m, this);
  }

  object_ptr obj::keyword::call(object_ptr const m, object_ptr const fallback) const
  {
    return runtime::get(m, this, fallback);
  }

  bool obj::keyword::operator==(obj::keyword const &rhs) const
  {
    return sym == rhs.sym;
  }
}
