#include <jank/util/scope_exit.hpp>

namespace jank::util
{
  scope_exit::scope_exit(scope_exit::function_type const &f)
    : func{ f }
  {
  }

  scope_exit::scope_exit(scope_exit::function_type &&f)
    : func{ std::move(f) }
  {
  }

  scope_exit::~scope_exit()
  {
    func();
  }
}
