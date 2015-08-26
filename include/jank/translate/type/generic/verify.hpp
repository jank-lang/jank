#pragma once

#include <jank/translate/cell/detail/type_definition.hpp>
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
          genericity<cell::detail::type_definition> const &expected,
          genericity<cell::detail::type_definition> const &provided
        );
      }
    }
  }
}
