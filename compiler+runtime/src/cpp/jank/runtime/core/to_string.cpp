#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::runtime
{
  native_persistent_string to_string(object const * const o)
  {
    return visit_object([](auto const typed_o) { return typed_o->to_string(); }, o);
  }

  void to_string(char const ch, util::string_builder &buff)
  {
    obj::character{ ch }.to_string(buff);
  }

  void to_string(object_ptr const o, util::string_builder &buff)
  {
    visit_object([&](auto const typed_o) { typed_o->to_string(buff); }, o);
  }

  native_persistent_string to_code_string(object const * const o)
  {
    return visit_object([](auto const typed_o) { return typed_o->to_code_string(); }, o);
  }

  void to_code_string(char const ch, util::string_builder &buff)
  {
    buff(obj::character{ ch }.to_code_string());
  }

  void to_code_string(object_ptr const o, util::string_builder &buff)
  {
    auto const value{ visit_object([](auto const typed_o) { return typed_o->to_code_string(); },
                                   o) };
    buff(value);
  }
}
