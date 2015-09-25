#pragma once

#include <boost/optional.hpp>

#include <jank/parse/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace type
    {
      namespace generic
      {
        /* Tries to find a generic specialization. Returns an it to the last
         * element of used such that ++it is the next element you want. */
        std::tuple
        <
          boost::optional<parse::cell::list>,
          parse::cell::list::type::const_iterator
        > extract
        (
          parse::cell::list::type::const_iterator const begin,
          parse::cell::list::type::const_iterator const end
        );
      }
    }
  }
}
