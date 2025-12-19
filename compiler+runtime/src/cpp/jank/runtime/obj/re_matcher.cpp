#include <jank/runtime/obj/re_matcher.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  re_matcher::re_matcher(re_pattern_ref const &re, jtl::immutable_string const &s)
    : re{ re }
    , match_input{ s.c_str() }
  {
  }

  bool re_matcher::equal(object const &o) const
  {
    return &base == &o;
  }

  void re_matcher::to_string(jtl::string_builder &buff) const
  {
    util::format_to(buff, "#object [{} {}]", object_type_str(base.type), &base);
  }

  jtl::immutable_string re_matcher::to_string() const
  {
    jtl::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  jtl::immutable_string re_matcher::to_code_string() const
  {
    return to_string();
  }

  uhash re_matcher::to_hash() const
  {
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
  }
}
