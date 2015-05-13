#pragma once

#include <jank/parse/cell/cell.hpp>

namespace jank
{
  namespace parse
  {
    namespace cell
    {
      namespace trait
      {
        inline type cell_to_enum(cell const &c)
        { return static_cast<type>(c.which()); }

        /* TODO: type_to_string */

        template <type C>
        char constexpr const* enum_to_string();
        template <>
        inline char constexpr const* enum_to_string<type::boolean>()
        { return "boolean"; }
        template <>
        inline char constexpr const* enum_to_string<type::integer>()
        { return "integer"; }
        template <>
        inline char constexpr const* enum_to_string<type::real>()
        { return "real"; }
        template <>
        inline char constexpr const* enum_to_string<type::string>()
        { return "string"; }
        template <>
        inline char constexpr const* enum_to_string<type::ident>()
        { return "ident"; }
        template <>
        inline char constexpr const* enum_to_string<type::list>()
        { return "list"; }
        template <>
        inline char constexpr const* enum_to_string<type::function>()
        { return "function"; }

        template <typename C>
        char constexpr const* type_to_string();
        template <>
        inline char constexpr const* type_to_string<boolean>()
        { return enum_to_string<type::boolean>(); }
        template <>
        inline char constexpr const* type_to_string<integer>()
        { return enum_to_string<type::integer>(); }
        template <>
        inline char constexpr const* type_to_string<real>()
        { return enum_to_string<type::real>(); }
        template <>
        inline char constexpr const* type_to_string<string>()
        { return enum_to_string<type::string>(); }
        template <>
        inline char constexpr const* type_to_string<ident>()
        { return enum_to_string<type::ident>(); }
        template <>
        inline char constexpr const* type_to_string<list>()
        { return enum_to_string<type::list>(); }
        template <>
        inline char constexpr const* type_to_string<function>()
        { return enum_to_string<type::function>(); }

        inline char constexpr const* enum_to_string(type const c)
        {
          switch(c)
          {
            case type::boolean:
              return enum_to_string<type::boolean>();
            case type::integer:
              return enum_to_string<type::integer>();
            case type::real:
              return enum_to_string<type::real>();
            case type::string:
              return enum_to_string<type::string>();
            case type::ident:
              return enum_to_string<type::ident>();
            case type::list:
              return enum_to_string<type::list>();
            case type::function:
              return enum_to_string<type::function>();
            default:
              return "unknown";
          }
        }

        /* TODO: this should be in env */
        inline type enum_from_string(std::string const &str)
        {
          if(str == "boolean")
          { return type::boolean; }
          else if(str == "integer")
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
          {
            throw interpret::expect::error::type::type<>
            { "invalid parse cell type string " + str };
          }
        }

        namespace detail
        {
          template <type C>
          struct enum_to_type;
          template <>
          struct enum_to_type<type::boolean>
          { using type = boolean; };
          template <>
          struct enum_to_type<type::integer>
          { using type = integer; };
          template <>
          struct enum_to_type<type::real>
          { using type = real; };
          template <>
          struct enum_to_type<type::string>
          { using type = string; };
          template <>
          struct enum_to_type<type::ident>
          { using type = ident; };
          template <>
          struct enum_to_type<type::list>
          { using type = list; };
          template <>
          struct enum_to_type<type::function>
          { using type = function; };
        }
        template <type C>
        using enum_to_type = typename detail::enum_to_type<C>::type;

        template <typename C>
        type constexpr type_to_enum();
        template <>
        type constexpr type_to_enum<boolean>()
        { return type::boolean; }
        template <>
        type constexpr type_to_enum<integer>()
        { return type::integer; }
        template <>
        type constexpr type_to_enum<real>()
        { return type::real; }
        template <>
        type constexpr type_to_enum<string>()
        { return type::string; }
        template <>
        type constexpr type_to_enum<ident>()
        { return type::ident; }
        template <>
        type constexpr type_to_enum<list>()
        { return type::list; }
        template <>
        type constexpr type_to_enum<function>()
        { return type::function; }
      }
    }
  }
}
