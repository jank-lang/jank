#include <fmt/format.h>

#include <jank/util/scope_exit.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/native_persistent_string/fmt.hpp>

namespace jank::util
{
  scope_exit::scope_exit(scope_exit::function_type &&f)
    : func{ std::move(f) }
  {
  }

  scope_exit::~scope_exit()
  {
    try
    {
      func();
    }
    catch(runtime::object_ptr const o)
    {
      /* TODO: Panic */
      fmt::println(stderr, "Exception caught in scope_exit: {}", runtime::to_string(o));
    }
    catch(std::exception const &e)
    {
      /* TODO: Panic */
      fmt::println(stderr, "Exception caught in scope_exit: {}", e.what());
    }
  }
}
