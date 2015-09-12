#pragma once

#include <jank/translate/cell/detail/type_definition.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        template <typename C>
        struct type_reference
        {
          type_definition<C> definition;
        };

        template <typename C>
        bool operator ==
        (type_reference<C> const &lhs, type_reference<C> const &rhs)
        { return lhs.definition == rhs.definition; }

        template <typename C>
        bool operator !=
        (type_reference<C> const &lhs, type_reference<C> const &rhs)
        { return lhs.definition != rhs.definition; }
      }
    }
  }
}
