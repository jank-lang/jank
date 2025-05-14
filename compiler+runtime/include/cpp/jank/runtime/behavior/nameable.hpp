#pragma once

namespace jank::runtime::behavior
{
  template <typename T>
  concept nameable = requires(T * const t) {
    { t->get_name() } -> std::convertible_to<jtl::immutable_string>;
    { t->get_namespace() } -> std::convertible_to<jtl::immutable_string>;
  };
}
