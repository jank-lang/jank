#pragma once

#include <memory>
#include <tuple>

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/cell.hpp>
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
        genericity<cell::detail::type_definition<cell::cell>> parse
        (
          parse::cell::list const &l,
          std::shared_ptr<environment::scope> const &scope
        );

        std::tuple
        <
          cell::detail::type_definition<cell::cell>,
          parse::cell::list::type::const_iterator
        > apply_genericity
        (
          cell::detail::type_definition<cell::cell> &&type,
          parse::cell::list::type::const_iterator const begin,
          parse::cell::list::type::const_iterator const end,
          std::shared_ptr<environment::scope> const &scope
        );
      }
    }
  }
}
