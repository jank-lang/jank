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
          template <typename C>
          struct argument_type
          {
            std::string name;
            cell::detail::type_reference<C> type;
          };
          template <typename C>
          using type_list = std::vector<argument_type<C>>;

          template <typename C>
          bool operator ==(type_list<C> const &lhs, type_list<C> const &rhs);

          template <typename C>
          std::ostream& operator <<(std::ostream &os, type_list<C> const &args);
        }
        template <typename C>
        using type_list = detail::type_list<C>;

        namespace definition
        {
          template <typename C>
          type_list<C> parse_types
          (
            parse::cell::list const &l,
            std::shared_ptr<environment::scope> const &scope
          );
        }
      }
    }
  }
}
