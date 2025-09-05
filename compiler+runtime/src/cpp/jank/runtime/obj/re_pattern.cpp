#include <jank/runtime/obj/re_pattern.hpp>
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
}
