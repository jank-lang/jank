#pragma once

#include <list>

#include <boost/variant.hpp>

#include <jank/parse/cell/cell.hpp>
#include <jank/parse/cell/trait.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      /* TODO: Rename literal_value to value? */
      enum class literal_type
      {
        null,
        boolean,
        integer,
        real,
        string,
        list
      };

      namespace detail
      {
        using literal_value = boost::variant
        <
          parse::cell::null,
          parse::cell::boolean,
          parse::cell::integer,
          parse::cell::real,
          parse::cell::string,
          std::list<parse::cell::integer> /* TODO */
        >;
      }
    }
  }
}
