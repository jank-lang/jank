#pragma once

#include <boost/variant.hpp>

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
        using literal_value = boost::variant
        <
          parse::cell::boolean::type,
          parse::cell::integer::type,
          parse::cell::real::type,
          parse::cell::string::type,
          parse::cell::ident::type
        >;
      }
    }
  }
}
