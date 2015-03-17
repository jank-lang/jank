#pragma once

#include <string>
#include <stdexcept>

#include <jank/interpret/expect/error/type/type.hpp>

namespace jank
{
  namespace cell
  {
    enum class type
    {
      integer,
      real,
      string,
      ident,
      list,
      function
    };

    template <type C>
    char constexpr const* type_string();
    template <>
    inline char constexpr const* type_string<type::integer>()
    { return "integer"; }
    template <>
    inline char constexpr const* type_string<type::real>()
    { return "real"; }
    template <>
    inline char constexpr const* type_string<type::string>()
    { return "string"; }
    template <>
    inline char constexpr const* type_string<type::ident>()
    { return "ident"; }
    template <>
    inline char constexpr const* type_string<type::list>()
    { return "list"; }
    template <>
    inline char constexpr const* type_string<type::function>()
    { return "function"; }

    /* TODO: constexpr */
    inline char const* type_string(type const c)
    {
      switch(c)
      {
        case type::integer:
          return type_string<type::integer>();
        case type::real:
          return type_string<type::real>();
        case type::string:
          return type_string<type::string>();
        case type::ident:
          return type_string<type::ident>();
        case type::list:
          return type_string<type::list>();
        case type::function:
          return type_string<type::function>();
        default:
          return "unknown";
      }
    }

    /* TODO: this should be in env */
    inline type type_from_string(std::string const &str)
    {
      if(str == "integer")
      { return type::integer; }
      else if(str == "real")
      { return type::real; }
      else if(str == "string")
      { return type::string; }
      else if(str == "ident")
      { return type::ident; }
      else if(str == "list")
      { return type::list; }
      else if(str == "function")
      { return type::function; }
      else
      { throw expect::error::type::type<>{ "invalid cell type string " + str }; }
    }
  }
}
