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

        /* TODO: GCC doesn't allow constexpr here yet. */
        inline char const* enum_to_string(type const c)
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

        namespace detail
        {
          template <typename C>
          struct type_to_enum;
          template <>
          struct type_to_enum<boolean>
          { static type constexpr value{ type::boolean }; };
          template <>
          struct type_to_enum<integer>
          { static type constexpr value{ type::integer }; };
          template <>
          struct type_to_enum<real>
          { static type constexpr value{ type::real }; };
          template <>
          struct type_to_enum<string>
          { static type constexpr value{ type::string }; };
          template <>
          struct type_to_enum<ident>
          { static type constexpr value{ type::ident }; };
          template <>
          struct type_to_enum<list>
          { static type constexpr value{ type::list }; };
          template <>
          struct type_to_enum<function>
          { static type constexpr value{ type::function }; };
        }
        template <typename C>
        using type_to_enum = typename detail::type_to_enum<C>::type;
      }
    }
  }
}
