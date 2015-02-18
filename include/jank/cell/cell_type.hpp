#pragma once

#include <string>
#include <stdexcept>

namespace jank
{
  namespace cell
  {
    enum class cell_type
    {
      integer,
      real,
      string,
      ident,
      list,
      function
    };

    template <cell_type C>
    char constexpr const* cell_type_string();
    template <>
    inline char constexpr const* cell_type_string<cell_type::integer>()
    { return "integer"; }
    template <>
    inline char constexpr const* cell_type_string<cell_type::real>()
    { return "real"; }
    template <>
    inline char constexpr const* cell_type_string<cell_type::string>()
    { return "string"; }
    template <>
    inline char constexpr const* cell_type_string<cell_type::ident>()
    { return "ident"; }
    template <>
    inline char constexpr const* cell_type_string<cell_type::list>()
    { return "list"; }
    template <>
    inline char constexpr const* cell_type_string<cell_type::function>()
    { return "function"; }

    /* TODO: constexpr */
    inline char const* cell_type_string(cell_type const c)
    {
      switch(c)
      {
        case cell_type::integer:
          return cell_type_string<cell_type::integer>();
        case cell_type::real:
          return cell_type_string<cell_type::real>();
        case cell_type::string:
          return cell_type_string<cell_type::string>();
        case cell_type::ident:
          return cell_type_string<cell_type::ident>();
        case cell_type::list:
          return cell_type_string<cell_type::list>();
        case cell_type::function:
          return cell_type_string<cell_type::function>();
        default:
          return "unknown";
      }
    }

    inline cell_type cell_type_from_string(std::string const &str)
    {
      if(str == "int")
      { return cell_type::integer; }
      else if(str == "real")
      { return cell_type::real; }
      else if(str == "string")
      { return cell_type::string; }
      else if(str == "ident")
      { return cell_type::ident; }
      else if(str == "list")
      { return cell_type::list; }
      else if(str == "function")
      { return cell_type::function; }
      else
      { throw std::runtime_error{ "invalid cell type string: " + str }; }
    }
  }
}
