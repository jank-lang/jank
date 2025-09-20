#pragma once

#include <jtl/immutable_string.hpp>
#include <jtl/option.hpp>

namespace jank::aot
{
  void register_resource(jtl::immutable_string const &name, jtl::immutable_string_view const &data);
  jtl::option<jtl::immutable_string_view> find_resource(jtl::immutable_string const &name);
}
