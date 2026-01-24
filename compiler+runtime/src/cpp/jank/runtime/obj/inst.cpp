#include <iostream>
#include <locale>
#include <sstream>

#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/obj/inst.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime::obj
{
  inst::inst()
    : object{ obj_type }
    , value{ std::chrono::system_clock::now() }
  {
  }

#ifdef _LIBCPP_VERSION
  /* The current version of libc++ within LLVM does not implement
   * std::chrono::parse() (a C++20 feature). Until it is added or an alternative
   * is used, inst parsing will be disabled
   *
   * To find progress for this feature in LLVM goto:
   * https://github.com/llvm/llvm-project/issues/99982
   */
  static inst_time_point inst_from_string(jtl::immutable_string const &)
  {
    throw make_box("'#inst' parsing not currently supported.").erase();
  }
#else
  static inst_time_point inst_from_string(jtl::immutable_string const &s)
  {
    static std::vector const formats{ "%FT%T%Oz", "%FT%T%z", "%FT%T", "%F" };

    inst_time_point o;
    bool parsed{ false };

    for(auto const format : formats)
    {
      std::istringstream is{ static_cast<std::string>(s) };
      is.imbue(std::locale("en_US.utf-8"));
      is >> std::chrono::parse(format, o);

      if(!is.fail() && is.peek() == EOF)
      {
        parsed = true;
        break;
      }
    }

    if(!parsed)
    {
      throw make_box(util::format("Unrecognized date/time syntax: {}", s)).erase();
    }

    return o;
  }
#endif

  inst::inst(jtl::immutable_string const &s)
    : object{ obj_type }
    , value{ inst_from_string(s) }
  {
  }

  bool inst::equal(object const &o) const
  {
    if(o.type != object_type::inst)
    {
      return false;
    }

    auto const s(expect_object<inst>(&o));
    return s->value == value;
  }

  static void to_string_impl(inst_time_point const &value, jtl::string_builder &buff)
  {
    buff(std::format("{:%FT%T}-00:00",
                     std::chrono::time_point_cast<std::chrono::milliseconds>(value)));
  }

  void inst::to_string(jtl::string_builder &buff) const
  {
    to_string_impl(value, buff);
  }

  jtl::immutable_string inst::to_string() const
  {
    jtl::string_builder buff;
    to_string_impl(value, buff);
    return buff.release();
  }

  jtl::immutable_string inst::to_code_string() const
  {
    jtl::string_builder buff;
    buff("#inst \"");
    to_string_impl(value, buff);
    buff("\"");
    return buff.release();
  }

  uhash inst::to_hash() const
  {
    return static_cast<uhash>(value.time_since_epoch().count());
  }
}
