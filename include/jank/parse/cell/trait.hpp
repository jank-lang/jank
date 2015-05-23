#pragma once

#include <jank/parse/cell/cell.hpp>
#include <jank/parse/expect/error/type/type.hpp>

namespace jank
{
  namespace parse
  {
    namespace cell
    {
      namespace trait
      {
        inline type to_enum(cell const &c)
        { return static_cast<type>(c.which()); }

        template <type C>
        char constexpr const* to_string();
        template <>
        inline char constexpr const* to_string<type::boolean>()
        { return "boolean"; }
        template <>
        inline char constexpr const* to_string<type::integer>()
        { return "integer"; }
        template <>
        inline char constexpr const* to_string<type::real>()
        { return "real"; }
        template <>
        inline char constexpr const* to_string<type::string>()
        { return "string"; }
        template <>
        inline char constexpr const* to_string<type::ident>()
        { return "ident"; }
        template <>
        inline char constexpr const* to_string<type::list>()
        { return "list"; }
        template <>
        inline char constexpr const* to_string<type::function>()
        { return "function"; }
        template <>
        inline char constexpr const* to_string<type::comment>()
        { return "comment"; }

        template <typename C>
        char constexpr const* to_string();
        template <>
        inline char constexpr const* to_string<boolean>()
        { return to_string<type::boolean>(); }
        template <>
        inline char constexpr const* to_string<integer>()
        { return to_string<type::integer>(); }
        template <>
        inline char constexpr const* to_string<real>()
        { return to_string<type::real>(); }
        template <>
        inline char constexpr const* to_string<string>()
        { return to_string<type::string>(); }
        template <>
        inline char constexpr const* to_string<ident>()
        { return to_string<type::ident>(); }
        template <>
        inline char constexpr const* to_string<list>()
        { return to_string<type::list>(); }
        template <>
        inline char constexpr const* to_string<function>()
        { return to_string<type::function>(); }
        template <>
        inline char constexpr const* to_string<comment>()
        { return to_string<type::comment>(); }

        inline char constexpr const* to_string(type const c)
        {
          switch(c)
          {
            case type::boolean:
              return to_string<type::boolean>();
            case type::integer:
              return to_string<type::integer>();
            case type::real:
              return to_string<type::real>();
            case type::string:
              return to_string<type::string>();
            case type::ident:
              return to_string<type::ident>();
            case type::list:
              return to_string<type::list>();
            case type::function:
              return to_string<type::function>();
            case type::comment:
              return to_string<type::comment>();
            default:
              return "unknown";
          }
        }

        inline type to_enum(std::string const &str)
        {
          if(str == to_string<boolean>())
          { return type::boolean; }
          else if(str == to_string<integer>())
          { return type::integer; }
          else if(str == to_string<real>())
          { return type::real; }
          else if(str == to_string<string>())
          { return type::string; }
          else if(str == to_string<ident>())
          { return type::ident; }
          else if(str == to_string<list>())
          { return type::list; }
          else if(str == to_string<function>())
          { return type::function; }
          else if(str == to_string<comment>())
          { return type::comment; }
          else
          {
            throw expect::error::type::exception<>
            { "invalid parse cell type string " + str };
          }
        }

        namespace detail
        {
          template <type C>
          struct to_type;
          template <>
          struct to_type<type::boolean>
          { using type = boolean; };
          template <>
          struct to_type<type::integer>
          { using type = integer; };
          template <>
          struct to_type<type::real>
          { using type = real; };
          template <>
          struct to_type<type::string>
          { using type = string; };
          template <>
          struct to_type<type::ident>
          { using type = ident; };
          template <>
          struct to_type<type::list>
          { using type = list; };
          template <>
          struct to_type<type::function>
          { using type = function; };
          template <>
          struct to_type<type::comment>
          { using type = comment; };
        }
        template <type C>
        using to_type = typename detail::to_type<C>::type;

        template <typename C>
        type constexpr to_enum();
        template <>
        type constexpr to_enum<boolean>()
        { return type::boolean; }
        template <>
        type constexpr to_enum<integer>()
        { return type::integer; }
        template <>
        type constexpr to_enum<real>()
        { return type::real; }
        template <>
        type constexpr to_enum<string>()
        { return type::string; }
        template <>
        type constexpr to_enum<ident>()
        { return type::ident; }
        template <>
        type constexpr to_enum<list>()
        { return type::list; }
        template <>
        type constexpr to_enum<function>()
        { return type::function; }
        template <>
        type constexpr to_enum<comment>()
        { return type::comment; }
      }
    }
  }
}
