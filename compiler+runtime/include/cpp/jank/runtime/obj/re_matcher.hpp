#pragma once

#include <regex>
#include <string>

#include <jank/runtime/obj/re_pattern.hpp>
#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using re_matcher_ref = oref<struct re_matcher>;

  struct re_matcher : object
  {
    static constexpr object_type obj_type{ object_type::re_matcher };
    static constexpr bool pointer_free{ false };

    re_matcher(re_pattern_ref const re, jtl::immutable_string const &s);

    /*** XXX: Everything here is immutable after initialization. ***/
    re_pattern_ref re;
    /* TODO: jtl::immutable_string here. */
    std::string match_input{};
    object_ref groups{};
  };
}
