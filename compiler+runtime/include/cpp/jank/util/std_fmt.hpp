#pragma once

#include <format>

#include <jtl/immutable_string.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/number.hpp>

namespace jank::runtime
{
  template <typename Out, typename... Args>
  void vformat_object_to(Out out,
                         jtl::immutable_string_view const fmt,
                         object_ref const arg,
                         Args &&...args)
  {
    visit_object(
      [&](auto const typed_o) {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(jtl::is_any_same<T, obj::integer, obj::small_integer>)
        {
          auto v{ typed_o->to_integer() };
          std::vformat_to(out, fmt, std::make_format_args(v, std::forward<Args>(args)...));
        }
        else if constexpr(jtl::is_any_same<T, obj::real, obj::small_real>)
        {
          auto v{ typed_o->to_real() };
          std::vformat_to(out, fmt, std::make_format_args(v, std::forward<Args>(args)...));
        }
        else
        {
          auto v{ typed_o->to_string() };
          std::string_view str{ v.view() };
          std::vformat_to(out, fmt, std::make_format_args(str, std::forward<Args>(args)...));
        }
      },
      arg);
  }
}
