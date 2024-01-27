#pragma once

#include <functional>

namespace jank::util
{
  struct scope_exit
  {
    using function_type = std::function<void ()>;

    scope_exit(function_type const &f);
    scope_exit(function_type &&f);
    ~scope_exit();

    function_type func;
  };
}
