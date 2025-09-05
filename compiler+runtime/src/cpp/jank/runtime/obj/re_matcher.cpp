#include <jank/runtime/obj/re_matcher.hpp>
#include <jank/runtime/rtti.hpp>

namespace jank::runtime::obj
{
  re_matcher::re_matcher(re_pattern_ref re, jtl::immutable_string const &s)
    : s{ s.c_str() }
    , re{ re }
  {
  }

  bool re_matcher::equal(object const &o) const
  {
    if(o.type != object_type::re_matcher)
    {
      return false;
    }

    auto const matcher(expect_object<re_matcher>(&o));
    return this == &(*matcher);
  }

  void re_matcher::to_string(jtl::string_builder &buff) const
  {
    buff("#<re_matcher>");
  }

  jtl::immutable_string re_matcher::to_string() const
  {
    jtl::string_builder buff;
    buff("#<re_matcher>");
    return buff.release();
  }

  jtl::immutable_string re_matcher::to_code_string() const
  {
    return to_string();
  }

  uhash re_matcher::to_hash() const
  {
    if(hash)
    {
      return hash;
    }

    // TODO: implement
    return 0;
  }
}
