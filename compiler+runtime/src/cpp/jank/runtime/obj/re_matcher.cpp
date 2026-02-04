#include <jank/runtime/obj/re_matcher.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  re_matcher::re_matcher(re_pattern_ref const re, jtl::immutable_string const &s)
    : object{ obj_type, obj_behaviors }
    , re{ re }
    , match_input{ s.c_str() }
  {
  }
}
