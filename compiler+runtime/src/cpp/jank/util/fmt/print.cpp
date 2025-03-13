#include <jank/util/fmt/print.hpp>

namespace jank::util
{
  void println(char const * const fmt)
  {
    std::fwrite(fmt, 1, strlen(fmt), stdout);
    std::putc('\n', stdout);
  }

  void println(FILE * const file, char const * const fmt)
  {
    std::fwrite(fmt, 1, strlen(fmt), file);
    std::putc('\n', file);
  }

  void print(char const * const fmt)
  {
    std::fwrite(fmt, 1, strlen(fmt), stdout);
  }

  void print(FILE * const file, char const * const fmt)
  {
    std::fwrite(fmt, 1, strlen(fmt), file);
  }
}
