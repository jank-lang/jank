#include <jank/translate/environment/builtin/value/primitive.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>

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
          cell::cell null()
          { return { cell::literal_value{ } }; }

          cell::cell boolean(bool const b)
          { return { cell::literal_value{ parse::cell::boolean{ b } } }; }

          cell::cell integer(parse::cell::integer::type const i)
          { return { cell::literal_value{ parse::cell::integer{ i } } }; }

          cell::cell real(parse::cell::real::type const r)
          { return { cell::literal_value{ parse::cell::real{ r } } }; }

          cell::cell string(parse::cell::string::type const &s)
          { return { cell::literal_value{ parse::cell::string{ s } } }; }
        }
      }
    }
  }
}
