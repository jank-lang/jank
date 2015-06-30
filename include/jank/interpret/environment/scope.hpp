#pragma once

#include <map>
#include <memory>
#include <string>
#include <experimental/optional>

#include <jank/parse/cell/cell.hpp>

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

        std::experimental::optional<parse::cell::cell> find_variable
        (std::string const &name) const;

        /* TODO: Use a custom interpret cell for these values. */
        std::map<std::string, parse::cell::cell> variables;
        std::shared_ptr<scope> parent;
      };
    }
  }
}
