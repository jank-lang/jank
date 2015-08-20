#pragma once

#include <map>
#include <memory>
#include <string>
#include <experimental/optional>

#include <jank/interpret/cell/cell.hpp>

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

        std::map<std::string, cell::cell> bindings;
        std::shared_ptr<scope> parent;
      };
    }
  }
}
