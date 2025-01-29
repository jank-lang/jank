#include <chrono>
#include <iostream>
#include <locale>
#include <sstream>

#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/obj/inst.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime::obj
{
  static jtl::ref<inst_time_point> now()
  {
    return jtl::make_ref<inst_time_point>(std::chrono::system_clock::now());
  }

  inst::inst()
    : value{ now() }
  {
  }

  static jtl::ref<inst_time_point> from_string(jtl::immutable_string const &s)
  {
    inst_time_point o;
    std::istringstream is{ static_cast<std::string>(s) };
    is.imbue(std::locale("en_US.utf-8"));
    is >> std::chrono::parse("%FT%T", o);

    if(is.fail())
    {
      throw make_box(util::format("Unrecognized date/time syntax: {}", s)).erase();
    }

    return jtl::make_ref<inst_time_point>(o);
  }

  inst::inst(jtl::immutable_string const &s)
    : value{ from_string(s) }
  {
  }

  bool inst::equal(object const &o) const
  {
    if(o.type != object_type::inst)
    {
      return false;
    }

    auto const s(expect_object<inst>(&o));
    return *s->value == *value;
  }

  void inst::to_string(jtl::string_builder &buff) const
  {
    buff("#inst \"");
    buff(std::format("{:%FT%T}", *value));
    buff("\"");
  }

  jtl::immutable_string inst::to_string() const
  {
    jtl::string_builder buff;
    buff("#inst \"");
    buff(std::format("{:%FT%T}", *value));
    buff("\"");
    return buff.release();
  }

  jtl::immutable_string inst::to_code_string() const
  {
    return to_string();
  }

  uhash inst::to_hash() const
  {
    if(hash)
    {
      return hash;
    }

    return hash = static_cast<uhash>(value->time_since_epoch().count());
  }
}
