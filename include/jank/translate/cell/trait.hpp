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
        inline type to_enum(cell const &c)
        { return static_cast<type>(c.which()); }

        template <type C>
        char constexpr const* to_string();
        template <>
        inline char constexpr const* to_string<type::function_body>()
        { return "function_body"; }
        template <>
        inline char constexpr const* to_string<type::function_definition>()
        { return "function_definition"; }
        template <>
        inline char constexpr const* to_string<type::function_call>()
        { return "function_call"; }
        template <>
        inline char constexpr const* to_string<type::type_definition>()
        { return "type_definition"; }
        template <>
        inline char constexpr const* to_string<type::type_reference>()
        { return "type_reference"; }
        template <>
        inline char constexpr const* to_string<type::variable_definition>()
        { return "variable_definition"; }
        template <>
        inline char constexpr const* to_string<type::variable_reference>()
        { return "variable_reference"; }
        template <>
        inline char constexpr const* to_string<type::literal_value>()
        { return "literal_value"; }
        template <>
        inline char constexpr const* to_string<type::return_statement>()
        { return "return_statement"; }

        template <typename C>
        char constexpr const* to_string();
        template <>
        inline char constexpr const* to_string<function_body>()
        { return to_string<type::function_body>(); }
        template <>
        inline char constexpr const* to_string<function_definition>()
        { return to_string<type::function_definition>(); }
        template <>
        inline char constexpr const* to_string<function_call>()
        { return to_string<type::function_call>(); }
        template <>
        inline char constexpr const* to_string<type_definition>()
        { return to_string<type::type_definition>(); }
        template <>
        inline char constexpr const* to_string<type_reference>()
        { return to_string<type::type_reference>(); }
        template <>
        inline char constexpr const* to_string<variable_definition>()
        { return to_string<type::variable_definition>(); }
        template <>
        inline char constexpr const* to_string<variable_reference>()
        { return to_string<type::variable_reference>(); }
        template <>
        inline char constexpr const* to_string<literal_value>()
        { return to_string<type::literal_value>(); }
        template <>
        inline char constexpr const* to_string<return_statement>()
        { return to_string<type::return_statement>(); }

        inline char constexpr const* to_string(type const c)
        {
          switch(c)
          {
            case type::function_body:
              return to_string<type::function_body>();
            case type::function_definition:
              return to_string<type::function_definition>();
            case type::function_call:
              return to_string<type::function_call>();
            case type::type_definition:
              return to_string<type::type_definition>();
            case type::type_reference:
              return to_string<type::type_reference>();
            case type::variable_definition:
              return to_string<type::variable_definition>();
            case type::variable_reference:
              return to_string<type::variable_reference>();
            case type::literal_value:
              return to_string<type::literal_value>();
            case type::return_statement:
              return to_string<type::return_statement>();
            default:
              return "unknown";
          }
        }

        inline type to_enum(std::string const &str)
        {
          if(str == to_string<function_body>())
          { return type::function_body; }
          else if(str == to_string<function_definition>())
          { return type::function_definition; }
          else if(str == to_string<function_call>())
          { return type::function_call; }
          else if(str == to_string<type_definition>())
          { return type::type_definition; }
          else if(str == to_string<type_reference>())
          { return type::type_reference; }
          else if(str == to_string<variable_definition>())
          { return type::variable_definition; }
          else if(str == to_string<variable_reference>())
          { return type::variable_reference; }
          else if(str == to_string<literal_value>())
          { return type::literal_value; }
          else if(str == to_string<return_statement>())
          { return type::return_statement; }
          else
          {
            throw expect::error::type::exception<>
            { "invalid translate cell type string " + str };
          }
        }

        namespace detail
        {
          template <type C>
          struct to_type;
          template <>
          struct to_type<type::function_body>
          { using type = function_body; };
          template <>
          struct to_type<type::function_definition>
          { using type = function_definition; };
          template <>
          struct to_type<type::function_call>
          { using type = function_call; };
          template <>
          struct to_type<type::type_definition>
          { using type = type_definition; };
          template <>
          struct to_type<type::type_reference>
          { using type = type_reference; };
          template <>
          struct to_type<type::variable_definition>
          { using type = variable_definition; };
          template <>
          struct to_type<type::variable_reference>
          { using type = variable_reference; };
          template <>
          struct to_type<type::literal_value>
          { using type = literal_value; };
          template <>
          struct to_type<type::return_statement>
          { using type = return_statement; };
        }
        template <type C>
        using to_type = typename detail::to_type<C>::type;

        template <typename C>
        type constexpr to_enum();
        template <>
        type constexpr to_enum<function_body>()
        { return type::function_body; }
        template <>
        type constexpr to_enum<function_definition>()
        { return type::function_definition; }
        template <>
        type constexpr to_enum<function_call>()
        { return type::function_call; }
        template <>
        type constexpr to_enum<type_definition>()
        { return type::type_definition; }
        template <>
        type constexpr to_enum<type_reference>()
        { return type::type_reference; }
        template <>
        type constexpr to_enum<variable_definition>()
        { return type::variable_definition; }
        template <>
        type constexpr to_enum<variable_reference>()
        { return type::variable_reference; }
        template <>
        type constexpr to_enum<literal_value>()
        { return type::literal_value; }
        template <>
        type constexpr to_enum<return_statement>()
        { return type::return_statement; }
      }
    }
  }
}
