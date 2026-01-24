#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime::obj
{
  keyword::keyword()
    : object{ obj_type }
  {
  }

  keyword::keyword(runtime::detail::must_be_interned, jtl::immutable_string_view const &s)
    : object{ obj_type }
    , sym{ make_box<obj::symbol>(s) }
  {
  }

  keyword::keyword(runtime::detail::must_be_interned,
                   jtl::immutable_string_view const &ns,
                   jtl::immutable_string_view const &n)
    : object{ obj_type }
    , sym{ make_box<obj::symbol>(ns, n) }
  {
  }

  static void to_string_impl(symbol const &sym, jtl::string_builder &buff)
  {
    std::back_inserter(buff) = ':';
    sym.to_string(buff);
  }

  void keyword::to_string(jtl::string_builder &buff) const
  {
    to_string_impl(*sym, buff);
  }

  jtl::immutable_string keyword::to_string() const
  {
    jtl::string_builder buff;
    to_string_impl(*sym, buff);
    return buff.release();
  }

  uhash keyword::to_hash() const
  {
    return sym->to_hash() + 0x9e3779b9;
  }

  i64 keyword::compare(object const &o) const
  {
    return compare(*expect_object<keyword>(&o));
  }

  i64 keyword::compare(keyword const &s) const
  {
    return sym->compare(*s.sym);
  }

  jtl::immutable_string const &keyword::get_name() const
  {
    return sym->name;
  }

  jtl::immutable_string const &keyword::get_namespace() const
  {
    return sym->ns;
  }

  object_ref keyword::call(object_ref const m)
  {
    return runtime::get(m, this);
  }

  object_ref keyword::call(object_ref const m, object_ref const fallback)
  {
    return runtime::get(m, this, fallback);
  }

  bool keyword::operator==(keyword const &rhs) const
  {
    return this == &rhs;
  }
}
