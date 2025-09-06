#pragma once

#include <chrono>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using inst_ref = oref<struct inst>;
  using inst_time_point = std::chrono::time_point<std::chrono::system_clock>;

  struct inst : object
  {
    static constexpr object_type obj_type{ object_type::inst };
    static constexpr bool pointer_free{ true };

    inst();
    inst(jtl::immutable_string const &s);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    object base{ obj_type };
    inst_time_point value;
    mutable uhash hash{};
  };
}
