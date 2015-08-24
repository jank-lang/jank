#pragma once

#include <vector>

#include <boost/variant.hpp>

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

        template <typename T>
        struct single
        {
          using type = T;
          type data;
        };

        template <typename T>
        struct tuple
        {
          using type = std::vector<T>;
          type data;
        };

        /* Cells can be types or values. */
        template <typename T> /* T = detail::type_definition */
        using parameter = boost::variant
        <
          single<T>,
          tuple<T>
        >;
      }
    }
  }
}
