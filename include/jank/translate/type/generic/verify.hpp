#pragma once

#include <jank/translate/cell/cell.hpp>
#include <jank/translate/type/generic/genericity.hpp>

namespace jank
{
  namespace translate
  {
    namespace type
    {
      namespace generic
      {
        void verify
        (
          genericity<cell::detail::type_definition<cell::cell>> const &expected,
          genericity<cell::detail::type_definition<cell::cell>> const &provided
        );
      }
    }
  }
}
