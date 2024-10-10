#include <jank/runtime/core.hpp>

namespace jank::runtime
{
  native_persistent_string to_string(object const * const o)
  {
    return visit_object([](auto const typed_o) { return typed_o->to_string(); }, o);
  }

  void to_string(char const ch, fmt::memory_buffer &buff)
  {
    obj::character{ ch }.to_string(buff);
  }

  void to_string(object_ptr const o, fmt::memory_buffer &buff)
  {
    visit_object([&](auto const typed_o) { typed_o->to_string(buff); }, o);
  }

  native_persistent_string to_code_string(object const * const o)
  {
    return visit_object([](auto const typed_o) { return typed_o->to_code_string(); }, o);
  }

  void to_code_string(char const ch, fmt::memory_buffer &buff)
  {
    fmt::format_to(std::back_inserter(buff), "{}", obj::character{ ch }.to_code_string());
  }

  void to_code_string(object_ptr const o, fmt::memory_buffer &buff)
  {
    auto const value = visit_object([](auto const typed_o) { return typed_o->to_code_string(); }, o);
    fmt::format_to(std::back_inserter(buff), "{}", value);
  }
}
