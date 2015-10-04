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
        { return "function-body"; }
        template <>
        inline char constexpr const* to_string<type::function_definition>()
        { return "function-definition"; }
        template <>
        inline char constexpr const* to_string<type::native_function_declaration>()
        { return "native-function-declaration"; }
        template <>
        inline char constexpr const* to_string<type::function_call>()
        { return "function-call"; }
        template <>
        inline char constexpr const* to_string<type::indirect_function_call>()
        { return "indirect-function-call"; }
        template <>
        inline char constexpr const* to_string<type::native_function_call>()
        { return "native-function-call"; }
        template <>
        inline char constexpr const* to_string<type::function_reference>()
        { return "function-reference"; }
        template <>
        inline char constexpr const* to_string<type::native_function_reference>()
        { return "native-function-reference"; }
        template <>
        inline char constexpr const* to_string<type::type_definition>()
        { return "type-definition"; }
        template <>
        inline char constexpr const* to_string<type::type_reference>()
        { return "type-reference"; }
        template <>
        inline char constexpr const* to_string<type::binding_definition>()
        { return "binding-definition"; }
        template <>
        inline char constexpr const* to_string<type::binding_reference>()
        { return "binding-reference"; }
        template <>
        inline char constexpr const* to_string<type::literal_value>()
        { return "literal-value"; }
        template <>
        inline char constexpr const* to_string<type::return_statement>()
        { return "return-statement"; }
        template <>
        inline char constexpr const* to_string<type::if_statement>()
        { return "if-statement"; }
        template <>
        inline char constexpr const* to_string<type::do_statement>()
        { return "do-statement"; }
        template <>
        inline char constexpr const* to_string<type::macro_definition>()
        { return "macro-definition"; }
        template <>
        inline char constexpr const* to_string<type::macro_call>()
        { return "macro-call"; }

        template <typename C>
        char constexpr const* to_string();
        template <>
        inline char constexpr const* to_string<function_body>()
        { return to_string<type::function_body>(); }
        template <>
        inline char constexpr const* to_string<function_definition>()
        { return to_string<type::function_definition>(); }
        template <>
        inline char constexpr const* to_string<native_function_declaration>()
        { return to_string<type::native_function_declaration>(); }
        template <>
        inline char constexpr const* to_string<function_call>()
        { return to_string<type::function_call>(); }
        template <>
        inline char constexpr const* to_string<indirect_function_call>()
        { return to_string<type::indirect_function_call>(); }
        template <>
        inline char constexpr const* to_string<native_function_call>()
        { return to_string<type::native_function_call>(); }
        template <>
        inline char constexpr const* to_string<function_reference>()
        { return to_string<type::function_reference>(); }
        template <>
        inline char constexpr const* to_string<native_function_reference>()
        { return to_string<type::native_function_reference>(); }
        template <>
        inline char constexpr const* to_string<type_definition>()
        { return to_string<type::type_definition>(); }
        template <>
        inline char constexpr const* to_string<type_reference>()
        { return to_string<type::type_reference>(); }
        template <>
        inline char constexpr const* to_string<binding_definition>()
        { return to_string<type::binding_definition>(); }
        template <>
        inline char constexpr const* to_string<binding_reference>()
        { return to_string<type::binding_reference>(); }
        template <>
        inline char constexpr const* to_string<literal_value>()
        { return to_string<type::literal_value>(); }
        template <>
        inline char constexpr const* to_string<return_statement>()
        { return to_string<type::return_statement>(); }
        template <>
        inline char constexpr const* to_string<if_statement>()
        { return to_string<type::if_statement>(); }
        template <>
        inline char constexpr const* to_string<do_statement>()
        { return to_string<type::do_statement>(); }
        template <>
        inline char constexpr const* to_string<macro_definition>()
        { return to_string<type::macro_definition>(); }
        template <>
        inline char constexpr const* to_string<macro_call>()
        { return to_string<type::macro_call>(); }

        inline char constexpr const* to_string(type const c)
        {
          switch(c)
          {
            case type::function_body:
              return to_string<type::function_body>();
            case type::function_definition:
              return to_string<type::function_definition>();
            case type::native_function_declaration:
              return to_string<type::native_function_declaration>();
            case type::function_call:
              return to_string<type::function_call>();
            case type::indirect_function_call:
              return to_string<type::indirect_function_call>();
            case type::native_function_call:
              return to_string<type::native_function_call>();
            case type::function_reference:
              return to_string<type::function_reference>();
            case type::native_function_reference:
              return to_string<type::native_function_reference>();
            case type::type_definition:
              return to_string<type::type_definition>();
            case type::type_reference:
              return to_string<type::type_reference>();
            case type::binding_definition:
              return to_string<type::binding_definition>();
            case type::binding_reference:
              return to_string<type::binding_reference>();
            case type::literal_value:
              return to_string<type::literal_value>();
            case type::return_statement:
              return to_string<type::return_statement>();
            case type::if_statement:
              return to_string<type::if_statement>();
            case type::do_statement:
              return to_string<type::do_statement>();
            case type::macro_definition:
              return to_string<type::macro_definition>();
            case type::macro_call:
              return to_string<type::macro_call>();
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
          else if(str == to_string<native_function_declaration>())
          { return type::native_function_declaration; }
          else if(str == to_string<function_call>())
          { return type::function_call; }
          else if(str == to_string<indirect_function_call>())
          { return type::indirect_function_call; }
          else if(str == to_string<native_function_call>())
          { return type::native_function_call; }
          else if(str == to_string<function_reference>())
          { return type::function_reference; }
          else if(str == to_string<native_function_reference>())
          { return type::native_function_reference; }
          else if(str == to_string<type_definition>())
          { return type::type_definition; }
          else if(str == to_string<type_reference>())
          { return type::type_reference; }
          else if(str == to_string<binding_definition>())
          { return type::binding_definition; }
          else if(str == to_string<binding_reference>())
          { return type::binding_reference; }
          else if(str == to_string<literal_value>())
          { return type::literal_value; }
          else if(str == to_string<return_statement>())
          { return type::return_statement; }
          else if(str == to_string<if_statement>())
          { return type::if_statement; }
          else if(str == to_string<do_statement>())
          { return type::do_statement; }
          else if(str == to_string<macro_definition>())
          { return type::macro_definition; }
          else if(str == to_string<macro_call>())
          { return type::macro_call; }
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
          struct to_type<type::native_function_declaration>
          { using type = native_function_declaration; };
          template <>
          struct to_type<type::function_call>
          { using type = function_call; };
          template <>
          struct to_type<type::indirect_function_call>
          { using type = indirect_function_call; };
          template <>
          struct to_type<type::native_function_call>
          { using type = native_function_call; };
          template <>
          struct to_type<type::function_reference>
          { using type = function_reference; };
          template <>
          struct to_type<type::native_function_reference>
          { using type = native_function_reference; };
          template <>
          struct to_type<type::type_definition>
          { using type = type_definition; };
          template <>
          struct to_type<type::type_reference>
          { using type = type_reference; };
          template <>
          struct to_type<type::binding_definition>
          { using type = binding_definition; };
          template <>
          struct to_type<type::binding_reference>
          { using type = binding_reference; };
          template <>
          struct to_type<type::literal_value>
          { using type = literal_value; };
          template <>
          struct to_type<type::return_statement>
          { using type = return_statement; };
          template <>
          struct to_type<type::if_statement>
          { using type = if_statement; };
          template <>
          struct to_type<type::do_statement>
          { using type = do_statement; };
          template <>
          struct to_type<type::macro_definition>
          { using type = macro_definition; };
          template <>
          struct to_type<type::macro_call>
          { using type = macro_call; };
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
        type constexpr to_enum<native_function_declaration>()
        { return type::native_function_declaration; }
        template <>
        type constexpr to_enum<function_call>()
        { return type::function_call; }
        template <>
        type constexpr to_enum<indirect_function_call>()
        { return type::indirect_function_call; }
        template <>
        type constexpr to_enum<native_function_call>()
        { return type::native_function_call; }
        template <>
        type constexpr to_enum<function_reference>()
        { return type::function_reference; }
        template <>
        type constexpr to_enum<native_function_reference>()
        { return type::native_function_reference; }
        template <>
        type constexpr to_enum<type_definition>()
        { return type::type_definition; }
        template <>
        type constexpr to_enum<type_reference>()
        { return type::type_reference; }
        template <>
        type constexpr to_enum<binding_definition>()
        { return type::binding_definition; }
        template <>
        type constexpr to_enum<binding_reference>()
        { return type::binding_reference; }
        template <>
        type constexpr to_enum<literal_value>()
        { return type::literal_value; }
        template <>
        type constexpr to_enum<return_statement>()
        { return type::return_statement; }
        template <>
        type constexpr to_enum<if_statement>()
        { return type::if_statement; }
        template <>
        type constexpr to_enum<do_statement>()
        { return type::do_statement; }
        template <>
        type constexpr to_enum<macro_definition>()
        { return type::macro_definition; }
        template <>
        type constexpr to_enum<macro_call>()
        { return type::macro_call; }
      }
    }
  }
}
