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
        inline type cell_to_enum(cell const &c)
        { return static_cast<type>(c.which()); }

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
        template <>
        inline char constexpr const* enum_to_string<type::type_definition>()
        { return "type_definition"; }
        template <>
        inline char constexpr const* enum_to_string<type::type_reference>()
        { return "type_reference"; }
        template <>
        inline char constexpr const* enum_to_string<type::variable_definition>()
        { return "variable_definition"; }
        template <>
        inline char constexpr const* enum_to_string<type::variable_reference>()
        { return "variable_reference"; }
        template <>
        inline char constexpr const* enum_to_string<type::literal_value>()
        { return "literal_value"; }

        template <typename C>
        char constexpr const* type_to_string();
        template <>
        inline char constexpr const* type_to_string<function_body>()
        { return enum_to_string<type::function_body>(); }
        template <>
        inline char constexpr const* type_to_string<function_definition>()
        { return enum_to_string<type::function_definition>(); }
        template <>
        inline char constexpr const* type_to_string<function_call>()
        { return enum_to_string<type::function_call>(); }
        template <>
        inline char constexpr const* type_to_string<type_definition>()
        { return enum_to_string<type::type_definition>(); }
        template <>
        inline char constexpr const* type_to_string<type_reference>()
        { return enum_to_string<type::type_reference>(); }
        template <>
        inline char constexpr const* type_to_string<variable_definition>()
        { return enum_to_string<type::variable_definition>(); }
        template <>
        inline char constexpr const* type_to_string<variable_reference>()
        { return enum_to_string<type::variable_reference>(); }
        template <>
        inline char constexpr const* type_to_string<literal_value>()
        { return enum_to_string<type::literal_value>(); }

        inline char constexpr const* enum_to_string(type const c)
        {
          switch(c)
          {
            case type::function_body:
              return enum_to_string<type::function_body>();
            case type::function_definition:
              return enum_to_string<type::function_definition>();
            case type::function_call:
              return enum_to_string<type::function_call>();
            case type::type_definition:
              return enum_to_string<type::type_definition>();
            case type::type_reference:
              return enum_to_string<type::type_reference>();
            case type::variable_definition:
              return enum_to_string<type::variable_definition>();
            case type::variable_reference:
              return enum_to_string<type::variable_reference>();
            case type::literal_value:
              return enum_to_string<type::literal_value>();
            default:
              return "unknown";
          }
        }

        /* TODO: Rename to 'to' not 'from'. */
        inline type enum_from_string(std::string const &str)
        {
          if(str == type_to_string<function_body>())
          { return type::function_body; }
          else if(str == type_to_string<function_definition>())
          { return type::function_definition; }
          else if(str == type_to_string<function_call>())
          { return type::function_call; }
          else if(str == type_to_string<type_definition>())
          { return type::type_definition; }
          else if(str == type_to_string<type_reference>())
          { return type::type_reference; }
          else if(str == type_to_string<variable_definition>())
          { return type::variable_definition; }
          else if(str == type_to_string<variable_reference>())
          { return type::variable_reference; }
          else if(str == type_to_string<literal_value>())
          { return type::literal_value; }
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
          template <>
          struct enum_to_type<type::type_definition>
          { using type = type_definition; };
          template <>
          struct enum_to_type<type::type_reference>
          { using type = type_reference; };
          template <>
          struct enum_to_type<type::variable_definition>
          { using type = variable_definition; };
          template <>
          struct enum_to_type<type::variable_reference>
          { using type = variable_reference; };
          template <>
          struct enum_to_type<type::literal_value>
          { using type = literal_value; };
        }
        template <type C>
        using enum_to_type = typename detail::enum_to_type<C>::type;

        template <typename C>
        type constexpr type_to_enum();
        template <>
        type constexpr type_to_enum<function_body>()
        { return type::function_body; }
        template <>
        type constexpr type_to_enum<function_definition>()
        { return type::function_definition; }
        template <>
        type constexpr type_to_enum<function_call>()
        { return type::function_call; }
        template <>
        type constexpr type_to_enum<type_definition>()
        { return type::type_definition; }
        template <>
        type constexpr type_to_enum<type_reference>()
        { return type::type_reference; }
        template <>
        type constexpr type_to_enum<variable_definition>()
        { return type::variable_definition; }
        template <>
        type constexpr type_to_enum<variable_reference>()
        { return type::variable_reference; }
        template <>
        type constexpr type_to_enum<literal_value>()
        { return type::literal_value; }
      }
    }
  }
}
