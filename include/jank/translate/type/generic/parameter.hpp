#pragma once

#include <vector>

#include <boost/variant.hpp>

#include <jank/translate/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    namespace type
    {
      namespace generic
      {
        enum class parameter_type
        {
          single,
          tuple
        };

        template <typename C>
        struct single
        {
          using type = C;
          type data;
        };

        template <typename C>
        struct tuple
        {
          using type = std::vector<C>;
          type data;
        };

        /* Cells can be types or values. */
        template <typename C>
        using parameter = boost::variant
        <
          single<C>,
          tuple<C>
        >;
      }
    }
  }
}
