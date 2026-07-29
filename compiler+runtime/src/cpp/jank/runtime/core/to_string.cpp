#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::runtime
{
  void to_string(char const ch, jtl::string_builder &buff)
  {
    obj::character{ ch }.to_string(buff);
  }

  void to_code_string(char const ch, jtl::string_builder &buff)
  {
    buff(obj::character{ ch }.to_code_string());
  }
}
