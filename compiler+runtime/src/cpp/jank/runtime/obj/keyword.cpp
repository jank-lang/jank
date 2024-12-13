#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime::obj
{
  keyword::keyword(detail::must_be_interned, native_persistent_string_view const &s)
    : sym{ s }
  {
  }

  keyword::keyword(detail::must_be_interned,
                   native_persistent_string_view const &ns,
                   native_persistent_string_view const &n)
    : sym{ ns, n }
  {
  }

  /* Keywords are interned, so we can always count on identity equality. */
  native_bool keyword::equal(object const &o) const
  {
    return &base == &o;
  }

  static void to_string_impl(symbol const &sym, fmt::memory_buffer &buff)
  {
    std::back_inserter(buff) = ':';
    sym.to_string(buff);
  }

  void keyword::to_string(fmt::memory_buffer &buff) const
  {
    to_string_impl(sym, buff);
  }

  native_persistent_string keyword::to_string() const
  {
    fmt::memory_buffer buff;
    to_string_impl(sym, buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_persistent_string keyword::to_code_string() const
  {
    return to_string();
  }

  native_hash keyword::to_hash() const
  {
    return sym.to_hash() + 0x9e3779b9;
  }

  native_integer keyword::compare(object const &o) const
  {
    return compare(*expect_object<keyword>(&o));
  }

  native_integer keyword::compare(keyword const &s) const
  {
    return sym.compare(s.sym);
  }

  native_persistent_string const &keyword::get_name() const
  {
    return sym.name;
  }

  native_persistent_string const &keyword::get_namespace() const
  {
    return sym.ns;
  }

  object_ptr keyword::call(object_ptr const m)
  {
    return runtime::get(m, this);
  }

  object_ptr keyword::call(object_ptr const m, object_ptr const fallback)
  {
    return runtime::get(m, this, fallback);
  }

  bool keyword::operator==(keyword const &rhs) const
  {
    return sym == rhs.sym;
  }
}
