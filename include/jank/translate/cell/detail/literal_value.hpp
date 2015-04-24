#pragma once

#include <jank/parse/cell/cell.hpp>
#include <jank/parse/cell/trait.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        /* TODO: Need a variant here; maybe parse::cell::cell. */
        template <parse::cell::type T>
        struct literal_value
        {
          std::string name;
          parse::cell::trait::enum_to_type<T> value;
        };
      }
    }
  }
}
