#pragma once

#include <jank/translate/environment/scope.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace builtin
      {
        namespace value
        {
          cell::cell null();
          cell::cell boolean(bool const b);
          cell::cell integer(parse::cell::integer::type const i);
          cell::cell real(parse::cell::real::type const i);
          cell::cell string(parse::cell::string::type const &i);
        }
      }
    }
  }
}
