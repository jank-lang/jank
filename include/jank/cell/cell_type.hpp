#pragma once

#include <string>
#include <stdexcept>

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
    char constexpr const* cell_type_string();
    template <>
    inline char constexpr const* cell_type_string<type::integer>()
    { return "integer"; }
    template <>
    inline char constexpr const* cell_type_string<type::real>()
    { return "real"; }
    template <>
    inline char constexpr const* cell_type_string<type::string>()
    { return "string"; }
    template <>
    inline char constexpr const* cell_type_string<type::ident>()
    { return "ident"; }
    template <>
    inline char constexpr const* cell_type_string<type::list>()
    { return "list"; }
    template <>
    inline char constexpr const* cell_type_string<type::function>()
    { return "function"; }

    /* TODO: constexpr */
    inline char const* cell_type_string(type const c)
    {
      switch(c)
      {
        case type::integer:
          return cell_type_string<type::integer>();
        case type::real:
          return cell_type_string<type::real>();
        case type::string:
          return cell_type_string<type::string>();
        case type::ident:
          return cell_type_string<type::ident>();
        case type::list:
          return cell_type_string<type::list>();
        case type::function:
          return cell_type_string<type::function>();
        default:
          return "unknown";
      }
    }

    inline type cell_type_from_string(std::string const &str)
    {
      if(str == "int")
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
      { throw std::runtime_error{ "invalid cell type string: " + str }; }
    }
  }
}
