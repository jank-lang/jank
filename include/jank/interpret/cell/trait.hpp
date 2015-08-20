#pragma once

#include <jank/parse/cell/trait.hpp>
#include <jank/interpret/cell/cell.hpp>
#include <jank/interpret/expect/error/internal/exception.hpp>

namespace jank
{
  namespace interpret
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
        inline char constexpr const* to_string<type::null>()
        { return parse::cell::trait::to_string<parse::cell::type::null>(); }
        template <>
        inline char constexpr const* to_string<type::boolean>()
        { return parse::cell::trait::to_string<parse::cell::type::boolean>(); }
        template <>
        inline char constexpr const* to_string<type::integer>()
        { return parse::cell::trait::to_string<parse::cell::type::integer>(); }
        template <>
        inline char constexpr const* to_string<type::real>()
        { return parse::cell::trait::to_string<parse::cell::type::real>(); }
        template <>
        inline char constexpr const* to_string<type::string>()
        { return parse::cell::trait::to_string<parse::cell::type::string>(); }
        template <>
        inline char constexpr const* to_string<type::function>()
        { return "function"; }

        template <typename C>
        char constexpr const* to_string();
        template <>
        inline char constexpr const* to_string<null>()
        { return to_string<type::null>(); }
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
        inline char constexpr const* to_string<function>()
        { return to_string<type::function>(); }

        inline char constexpr const* to_string(type const c)
        {
          switch(c)
          {
            case type::null:
              return to_string<type::null>();
            case type::boolean:
              return to_string<type::boolean>();
            case type::integer:
              return to_string<type::integer>();
            case type::real:
              return to_string<type::real>();
            case type::string:
              return to_string<type::string>();
            case type::function:
              return to_string<type::function>();
            default:
              return "unknown";
          }
        }

        inline type to_enum(std::string const &str)
        {
          if(str == to_string<null>())
          { return type::null; }
          else if(str == to_string<boolean>())
          { return type::boolean; }
          else if(str == to_string<integer>())
          { return type::integer; }
          else if(str == to_string<real>())
          { return type::real; }
          else if(str == to_string<string>())
          { return type::string; }
          else if(str == to_string<function>())
          { return type::function; }
          else
          {
            throw expect::error::internal::exception<>
            { "invalid interpret cell type string " + str };
          }
        }

        namespace detail
        {
          template <type C>
          struct to_type;
          template <>
          struct to_type<type::null>
          { using type = null; };
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
          struct to_type<type::function>
          { using type = function; };
        }
        template <type C>
        using to_type = typename detail::to_type<C>::type;

        template <typename C>
        type constexpr to_enum();
        template <>
        type constexpr to_enum<null>()
        { return type::null; }
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
        type constexpr to_enum<function>()
        { return type::function; }
      }
    }
  }
}
