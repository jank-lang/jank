#include <jank/runtime/obj/regex.hpp>
#include <jank/runtime/rtti.hpp>

namespace jank::runtime::obj
{

  re_pattern::re_pattern(jtl::immutable_string const &s)
    : pattern{ s }
    , regex{ s.c_str(), std::regex_constants::ECMAScript }
  {
  }

  bool re_pattern::equal(object const &o) const
  {
    if(o.type != object_type::re_pattern)
    {
      return false;
    }

    auto const re(expect_object<re_pattern>(&o));
    return re->pattern == pattern;
  }

  void re_pattern::to_string(jtl::string_builder &buff) const
  {
    buff(pattern);
  }

  jtl::immutable_string re_pattern::to_string() const
  {
    return pattern;
  }

  jtl::immutable_string re_pattern::to_code_string() const
  {
    jtl::string_builder buff;
    buff("#\"");
    buff(pattern);
    buff('"');
    return buff.release();
  }

  uhash re_pattern::to_hash() const
  {
    if(hash)
    {
      return hash;
    }

    return hash = pattern.to_hash();
  }

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
