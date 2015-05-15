#pragma once

#include <string>
#include <vector>
#include <memory>

#include <jank/parse/cell/cell.hpp>

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
          struct argument_value
          {
            std::string name;
            C cell;
          };
          template <typename C>
          using value_list = typename std::vector<argument_value<C>>;
        }
        template <typename C>
        using value_list = typename detail::value_list<C>;

        namespace call
        {
          template <typename C>
          value_list<C> parse
          (
            parse::cell::list const &l,
            std::shared_ptr<environment::scope> const &scope
          );
        }
      }
    }
  }
}
