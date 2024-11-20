#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::perf
{
  object_ptr benchmark(object_ptr label, object_ptr f);
}
