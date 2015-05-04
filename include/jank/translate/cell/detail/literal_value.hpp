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
          parse::cell::boolean,
          parse::cell::integer,
          parse::cell::real,
          parse::cell::string,
          parse::cell::ident
        >;
      }
    }
  }
}
