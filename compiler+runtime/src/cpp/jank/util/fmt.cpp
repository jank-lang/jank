#include <jank/util/fmt.hpp>

namespace jank::util
{
  native_persistent_string format(char const * const fmt)
  {
    string_builder sb;
    sb(fmt);
    return sb.release();
  }

  void format(string_builder &sb, char const * const fmt)
  {
    sb(fmt);
  }
}
