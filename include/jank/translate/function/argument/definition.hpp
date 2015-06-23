#pragma once

#include <string>
#include <vector>
#include <ostream>
#include <memory>

#include <jank/parse/cell/cell.hpp>
#include <jank/parse/cell/visit.hpp>
#include <jank/parse/cell/trait.hpp>
#include <jank/translate/cell/type.hpp>
#include <jank/translate/cell/detail/type_reference.hpp>
#include <jank/translate/expect/error/type/exception.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    { struct scope; }

    namespace function
    {
      namespace argument
      {
        namespace detail
        {
          struct argument_type
          {
            std::string name;
            cell::detail::type_reference type;
          };
          using type_list = std::vector<argument_type>;

          bool operator ==(type_list const &lhs, type_list const &rhs);
          std::ostream& operator <<(std::ostream &os, type_list const &args);
        }
        using type_list = detail::type_list;

        namespace definition
        {
          type_list parse_types
          (
            parse::cell::list const &l,
            std::shared_ptr<environment::scope> const &scope
          );
        }
      }
    }
  }
}
