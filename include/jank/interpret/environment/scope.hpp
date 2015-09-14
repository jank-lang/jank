#pragma once

#include <map>
#include <memory>
#include <string>
#include <experimental/optional>

#include <jank/translate/cell/cell.hpp>
#include <jank/interpret/plugin/detail/native_function_definition.hpp>

namespace jank
{
  namespace interpret
  {
    namespace environment
    {
      struct scope
      {
        scope() = default;
        explicit scope(std::shared_ptr<scope> const &p)
          : parent{ p }
        { }

        std::experimental::optional<cell::cell> find_binding
        (std::string const &name) const;
        std::experimental::optional<plugin::detail::native_function_definition>
        find_native_function
        (
          translate::cell::detail::native_function_declaration
          <translate::cell::cell> const &
        ) const;

        std::map<std::string, cell::cell> bindings;
        std::map
        <
          translate::cell::detail::native_function_declaration
          <translate::cell::cell>,
          plugin::detail::native_function_definition
        > native_function_definitions;
        std::shared_ptr<scope> parent;
      };
    }
  }
}
