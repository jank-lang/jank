#include <jank/util/fmt.hpp>

namespace jank::util
{
  jtl::immutable_string format(char const * const fmt)
  {
    if(native_persistent_string_view{ fmt }.find("{}") != native_persistent_string_view::npos)
    {
      throw std::runtime_error{ "Format string has extra {} with no matching argument" };
    }

    string_builder sb;
    sb(fmt);
    return sb.release();
  }

  void format_to(string_builder &sb, char const * const fmt)
  {
    if(native_persistent_string_view{ fmt }.find("{}") != native_persistent_string_view::npos)
    {
      throw std::runtime_error{ "Format string has extra {} with no matching argument" };
    }

    sb(fmt);
  }
}
