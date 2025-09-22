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
    return &base == &o;
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
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
  }
}
