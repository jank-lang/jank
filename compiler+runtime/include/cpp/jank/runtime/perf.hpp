#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::perf
{
  object_ref benchmark(object_ref const opts, object_ref const f);
}
