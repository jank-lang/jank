#pragma once

namespace jank
{
  int init(int const argc,
           char const **argv,
           bool init_default_ctx,
           void (*fn)(int const, char const **));
}
