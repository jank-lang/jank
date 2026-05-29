#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::runtime
{
  jtl::immutable_string to_string(object_ref const o)
  {
    return o.to_string();
  }

  void to_string(char const ch, jtl::string_builder &buff)
  {
    obj::character{ ch }.to_string(buff);
  }

  void to_string(object_ref const o, jtl::string_builder &buff)
  {
    o.to_string(buff);
  }

  jtl::immutable_string to_code_string(object_ref const o)
  {
    return o.to_code_string();
  }

  void to_code_string(char const ch, jtl::string_builder &buff)
  {
    buff(obj::character{ ch }.to_code_string());
  }

  void to_code_string(object_ref const o, jtl::string_builder &buff)
  {
    auto const value{ o.to_code_string() };
    buff(value);
  }
}
