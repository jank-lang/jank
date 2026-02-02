#pragma once

#include <functional>

namespace jank::util
{
  struct scope_exit
  {
    using function_type = std::function<void()>;

    scope_exit(function_type &&f);
    scope_exit(function_type &&f, bool should_propagate_exceptions);
    ~scope_exit() noexcept(false);

    void release()
    {
      active = false;
    }

    function_type func;
    bool should_propagate_exceptions{ false };
    bool active{ true };
  };
}
