#pragma once

#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace trait
      {
        template <type C>
        char constexpr const* enum_to_string();
        template <>
        inline char constexpr const* enum_to_string<type::function_body>()
        { return "function_body"; }
        template <>
        inline char constexpr const* enum_to_string<type::function_definition>()
        { return "function_definition"; }
        template <>
        inline char constexpr const* enum_to_string<type::function_call>()
        { return "function_call"; }

        /* TODO: GCC doesn't allow constexpr here yet. */
        inline char const* enum_to_string(type const c)
        {
          switch(c)
          {
            case type::function_body:
              return enum_to_string<type::function_body>();
            case type::function_definition:
              return enum_to_string<type::function_definition>();
            case type::function_call:
              return enum_to_string<type::function_call>();
            default:
              return "unknown";
          }
        }

        /* TODO: This should be in env. */
        inline type enum_from_string(std::string const &str)
        {
          if(str == "function_body")
          { return type::function_body; }
          else if(str == "function_definition")
          { return type::function_definition; }
          else if(str == "function_call")
          { return type::function_call; }
          else
          {
            throw interpret::expect::error::type::type<>
            { "invalid translate cell type string " + str };
          }
        }

        namespace detail
        {
          template <type C>
          struct enum_to_type;
          template <>
          struct enum_to_type<type::function_body>
          { using type = function_body; };
          template <>
          struct enum_to_type<type::function_definition>
          { using type = function_definition; };
          template <>
          struct enum_to_type<type::function_call>
          { using type = function_call; };
        }
        template <type C>
        using enum_to_type = typename detail::enum_to_type<C>::type;

        namespace detail
        {
          template <typename C>
          struct type_to_enum;
          template <>
          struct type_to_enum<function_body>
          { static type constexpr value{ type::function_body }; };
          template <>
          struct type_to_enum<function_definition>
          { static type constexpr value{ type::function_definition }; };
          template <>
          struct type_to_enum<function_call>
          { static type constexpr value{ type::function_call }; };
        }
        template <typename C>
        using type_to_enum = typename detail::type_to_enum<C>::type;
      }
    }
  }
}
