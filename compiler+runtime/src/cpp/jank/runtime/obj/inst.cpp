#include <array>
#include <cctype>
#include <iostream>
#include <locale>
#include <sstream>
#include <string_view>

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

  namespace
  {
    [[noreturn]]
    void throw_inst_parse_error(jtl::immutable_string const &input)
    {
      throw make_box(util::format("Unrecognized date/time syntax: {}", input)).erase();
    }

#ifdef _LIBCPP_VERSION

    bool consume_if(std::string_view &remaining, char const expected)
    {
      if(!remaining.empty() && remaining.front() == expected)
      {
        remaining.remove_prefix(1);
        return true;
      }
      return false;
    }

    bool starts_with_digits(std::string_view const &remaining, size_t const digits)
    {
      if(remaining.size() < digits)
      {
        return false;
      }
      for(size_t i{}; i < digits; ++i)
      {
        if(!std::isdigit(static_cast<unsigned char>(remaining[i])))
        {
          return false;
        }
      }
      return true;
    }

    int parse_fixed_digits(std::string_view &remaining,
                           size_t const digits,
                           jtl::immutable_string const &original)
    {
      if(!starts_with_digits(remaining, digits))
      {
        throw_inst_parse_error(original);
      }

      int value{};
      for(size_t i{}; i < digits; ++i)
      {
        auto const c(static_cast<unsigned char>(remaining[i]));
        value = (value * 10) + (c - '0');
      }
      remaining.remove_prefix(digits);
      return value;
    }

    int parse_fractional_millis(std::string_view &remaining, jtl::immutable_string const &original)
    {
      if(!consume_if(remaining, '.'))
      {
        return 0;
      }

      int value{};
      size_t stored_digits{};
      size_t total_digits{};
      while(!remaining.empty() && std::isdigit(static_cast<unsigned char>(remaining.front())))
      {
        if(stored_digits < 9)
        {
          auto const digit(static_cast<unsigned char>(remaining.front()) - '0');
          value = (value * 10) + digit;
          ++stored_digits;
        }
        ++total_digits;
        remaining.remove_prefix(1);
      }

      if(total_digits == 0)
      {
        throw_inst_parse_error(original);
      }

      if(stored_digits == 0)
      {
        return 0;
      }

      if(stored_digits > 3)
      {
        for(size_t i = stored_digits; i > 3; --i)
        {
          value /= 10;
        }
      }
      else
      {
        for(size_t i = stored_digits; i < 3; ++i)
        {
          value *= 10;
        }
      }

      return value;
    }

    int parse_timezone_offset(std::string_view &remaining, jtl::immutable_string const &original)
    {
      if(remaining.empty())
      {
        return 0;
      }

      auto const indicator(remaining.front());
      if(indicator == 'Z' || indicator == 'z')
      {
        remaining.remove_prefix(1);
        return 0;
      }

      if(indicator != '+' && indicator != '-')
      {
        throw_inst_parse_error(original);
      }

      auto const sign(indicator == '-' ? -1 : 1);
      remaining.remove_prefix(1);
      auto const tz_hour(parse_fixed_digits(remaining, 2, original));
      int tz_minute{};

      if(consume_if(remaining, ':'))
      {
        tz_minute = parse_fixed_digits(remaining, 2, original);
      }
      else if(starts_with_digits(remaining, 2))
      {
        tz_minute = parse_fixed_digits(remaining, 2, original);
      }

      if(tz_hour > 23 || tz_minute > 59)
      {
        throw_inst_parse_error(original);
      }

      return sign * ((tz_hour * 60) + tz_minute);
    }
  }

  static inst_time_point inst_from_string(jtl::immutable_string const &s)
  {
    using namespace std::chrono;

    std::string_view remaining{ s.c_str(), s.size() };

    auto const year(parse_fixed_digits(remaining, 4, s));

    int month{ 1 };
    int day{ 1 };
    if(consume_if(remaining, '-'))
    {
      month = parse_fixed_digits(remaining, 2, s);
      if(consume_if(remaining, '-'))
      {
        day = parse_fixed_digits(remaining, 2, s);
      }
    }

    int hour{};
    int minute{};
    int second{};
    int millisecond{};

    if(!remaining.empty() && (remaining.front() == 'T' || remaining.front() == 't'))
    {
      remaining.remove_prefix(1);
      hour = parse_fixed_digits(remaining, 2, s);
      if(!consume_if(remaining, ':'))
      {
        throw_inst_parse_error(s);
      }
      minute = parse_fixed_digits(remaining, 2, s);
      if(!consume_if(remaining, ':'))
      {
        throw_inst_parse_error(s);
      }
      second = parse_fixed_digits(remaining, 2, s);
      millisecond = parse_fractional_millis(remaining, s);
    }

    if(hour > 23 || minute > 59 || second > 59)
    {
      throw_inst_parse_error(s);
    }

    auto const offset_minutes(parse_timezone_offset(remaining, s));

    if(!remaining.empty())
    {
      throw_inst_parse_error(s);
    }

    auto const ymd(std::chrono::year{ year } / std::chrono::month{ static_cast<unsigned>(month) }
                   / std::chrono::day{ static_cast<unsigned>(day) });
    if(!ymd.ok())
    {
      throw_inst_parse_error(s);
    }

    auto const day_point(std::chrono::sys_days{ ymd });
    auto sys_time_ms(std::chrono::sys_time<std::chrono::milliseconds>{
      std::chrono::duration_cast<std::chrono::milliseconds>(day_point.time_since_epoch()) });
    sys_time_ms += std::chrono::hours{ hour } + std::chrono::minutes{ minute }
      + std::chrono::seconds{ second } + std::chrono::milliseconds{ millisecond };
    sys_time_ms -= std::chrono::minutes{ offset_minutes };

    return inst_time_point{ std::chrono::time_point_cast<inst_time_point::duration>(sys_time_ms) };
  }
#else
    static inst_time_point inst_from_string(jtl::immutable_string const &s)
    {
      constexpr std::array formats{ "%FT%T%Oz", "%FT%T%z", "%FT%T", "%F", "%Y-%m", "%Y" };

      inst_time_point parsed_value;
      for(auto const *format : formats)
      {
        std::istringstream is{ static_cast<std::string>(s) };
        is.imbue(std::locale("en_US.utf-8"));
        is >> std::chrono::parse(format, parsed_value);

        if(!is.fail() && is.peek() == EOF)
        {
          return parsed_value;
        }
      }

      throw_inst_parse_error(s);
    }
#endif

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
