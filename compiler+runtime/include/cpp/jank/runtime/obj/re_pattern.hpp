#pragma once

#include <regex>
#include <string>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using re_pattern_ref = oref<struct re_pattern>;

  struct re_pattern : object
  {
    static constexpr object_type obj_type{ object_type::re_pattern };
    static constexpr bool pointer_free{ false };

    re_pattern(jtl::immutable_string const &s);

    /* behavior::object_like */
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;

    /*** XXX: Everything here is immutable after initialization. ***/
    jtl::immutable_string pattern{};
    std::regex regex{};
  };
}
