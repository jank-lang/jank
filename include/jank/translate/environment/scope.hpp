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
      struct scope
      {
        scope() = default;
        explicit scope(std::shared_ptr<scope> const &p)
          : parent{ p }
        { }

        std::experimental::optional<std::vector<cell::function_definition>> find_function
        (std::string const &name);

        std::map<std::string, std::vector<cell::function_definition>> function_definitions;
        std::shared_ptr<scope> parent{ std::make_shared<scope>() };
      };
    }
  }
}
