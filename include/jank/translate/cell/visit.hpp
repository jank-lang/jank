#pragma once

#include <jank/translate/cell/cell.hpp>
#include <jank/translate/cell/trait.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      /* Follows the same pattern as parse::cell::visit. */
      template <typename Cell, typename Function>
      auto visit(Cell &&c, Function const &func)
      {
        switch(trait::to_enum(c))
        {
          case type::function_body:
            return func(boost::get<function_body>(c));
          case type::function_definition:
            return func(boost::get<function_definition>(c));
          case type::native_function_declaration:
            return func(boost::get<native_function_declaration>(c));
          case type::function_call:
            return func(boost::get<function_call>(c));
          case type::indirect_function_call:
            return func(boost::get<indirect_function_call>(c));
          case type::native_function_call:
            return func(boost::get<native_function_call>(c));
          case type::function_reference:
            return func(boost::get<function_reference>(c));
          case type::native_function_reference:
            return func(boost::get<native_function_reference>(c));
          case type::type_definition:
            return func(boost::get<type_definition>(c));
          case type::type_reference:
            return func(boost::get<type_reference>(c));
          case type::binding_definition:
            return func(boost::get<binding_definition>(c));
          case type::binding_reference:
            return func(boost::get<binding_reference>(c));
          case type::literal_value:
            return func(boost::get<literal_value>(c));
          case type::return_statement:
            return func(boost::get<return_statement>(c));
          case type::if_statement:
            return func(boost::get<if_statement>(c));
          case type::do_statement:
            return func(boost::get<do_statement>(c));
          case type::macro_definition:
            return func(boost::get<macro_definition>(c));
          case type::macro_call:
            return func(boost::get<macro_call>(c));
          default:
            throw std::runtime_error{ "invalid visit on translation cell" };
        }
      }
    }
  }
}
