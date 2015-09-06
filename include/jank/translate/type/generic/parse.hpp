#pragma once

#include <memory>
#include <tuple>

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

        std::tuple
        <
          cell::detail::type_definition,
          parse::cell::list::type::const_iterator
        > apply_genericity
        (
          cell::detail::type_definition &&type,
          parse::cell::list::type::const_iterator const begin,
          parse::cell::list::type::const_iterator const end,
          std::shared_ptr<environment::scope> const &scope
        );
      }
    }
  }
}
