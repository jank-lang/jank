#pragma once

#include <map>
#include <string>
#include <memory>
#include <experimental/optional>

#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      /* TODO: Have a shared_scope_ptr or scope_ptr here and in interpret. */
      struct scope : std::enable_shared_from_this<scope>
      {
        template <typename T>
        using result = std::pair<T, std::shared_ptr<scope const>>;

        scope() = default;
        explicit scope(std::shared_ptr<scope> const &p)
          : parent{ p }
        { }

        std::experimental::optional<result<cell::type_definition>> find_type
        (std::string const &name) const;
        std::experimental::optional<result<cell::binding_definition>> find_binding
        (std::string const &name) const;
        std::experimental::optional<std::vector<result<cell::function_definition>>> find_function
        (std::string const &name) const;
        std::experimental::optional<std::vector<result<cell::native_function_definition>>> find_native_function
        (std::string const &name) const;

        cell::function_body::type expect_function(cell::function_definition::type const &def);

        std::map<std::string, cell::type_definition> type_definitions;
        std::map<std::string, cell::binding_definition> binding_definitions;
        std::map<std::string, std::vector<cell::function_definition>> function_definitions;
        std::map<std::string, std::vector<cell::native_function_definition>> native_function_definitions;
        std::shared_ptr<scope> parent;
      };
    }
  }
}
