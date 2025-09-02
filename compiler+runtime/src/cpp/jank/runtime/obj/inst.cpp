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
    : value{ std::chrono::system_clock::now() }
  {
  }

  static inst_time_point inst_from_string(jtl::immutable_string const &s)
  {
    inst_time_point o;
    auto const formats = { "%FT%T%Oz", "%FT%T%z", "%FT%T", "%F" };
    auto parsed = false;

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

  inst::inst(jtl::immutable_string const &s)
    : value{ inst_from_string(s) }
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
    if(hash)
    {
      return hash;
    }

    return hash = static_cast<uhash>(value.time_since_epoch().count());
  }
}
