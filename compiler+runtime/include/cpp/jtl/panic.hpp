#pragma once

#include <jank/util/fmt.hpp>
#include <jtl/trait/transform.hpp>
#include <jtl/memory.hpp>

namespace jtl
{
  namespace detail
  {
    void panic(char const *msg);
  }

  template <typename... Args>
  [[noreturn]]
  void panic(char const * const fmt, Args &&...args) noexcept
  {
    auto const &msg{ jank::util::format(fmt, jtl::forward<Args>(args)...) };
    detail::panic(msg.c_str());
  }
}
