#pragma once

#include <regex>
#include <string>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using re_pattern_ref = oref<struct re_pattern>;

  struct re_pattern : gc
  {
    static constexpr object_type obj_type{ object_type::re_pattern };
    static constexpr bool pointer_free{ false };

    re_pattern(jtl::immutable_string const &s);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(jtl::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    object base{ obj_type };

    jtl::immutable_string pattern{};
    std::regex regex{};

    mutable uhash hash{};
  };

  using re_matcher_ref = oref<struct re_matcher>;

  struct re_matcher : gc
  {
    static constexpr object_type obj_type{ object_type::re_matcher };
    static constexpr bool pointer_free{ false };

    re_matcher(re_pattern_ref re, jtl::immutable_string const &s);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(jtl::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    object base{ obj_type };

    std::string s{};
    std::smatch m{};
    re_pattern_ref re;

    mutable uhash hash{};
  };
}
