#pragma once

#include <regex>
#include <string>

#include <jank/runtime/obj/re_pattern.hpp>
#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using re_matcher_ref = oref<struct re_matcher>;

  struct re_matcher
  {
    static constexpr object_type obj_type{ object_type::re_matcher };
    static constexpr bool pointer_free{ false };

    re_matcher(re_pattern_ref const re, jtl::immutable_string const &s);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(jtl::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    object base{ obj_type };

    re_pattern_ref re;
    std::string match_input{};
    object_ref groups{};
  };
}
