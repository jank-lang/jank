#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::perf
{
  object_ref benchmark(object_ref opts, object_ref f);
}
