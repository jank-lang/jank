#pragma once

#include <memory>

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/detail/type_definition.hpp>
#include <jank/translate/type/generic/genericity.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    { struct scope; }

    namespace type
    {
      namespace generic
      {
        genericity<cell::detail::type_definition> parse
        (
          parse::cell::list const &l,
          std::shared_ptr<environment::scope> const &scope
        );
      }
    }
  }
}
