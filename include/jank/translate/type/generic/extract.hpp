#pragma once

#include <jank/parse/cell/cell.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/translate/expect/error/type/invalid_generic.hpp>

namespace jank
{
  namespace translate
  {
    namespace type
    {
      namespace generic
      {
        /* Tries to find a generic specialization. */
        inline std::tuple
        <
          std::experimental::optional<parse::cell::list>,
          parse::cell::list::type::const_iterator
        > extract /* TODO: move to cpp */
        (
          parse::cell::list::type::const_iterator begin,
          parse::cell::list::type::const_iterator const end
        )
        {
          auto const &colon
          (parse::expect::optional_cast<parse::cell::type::ident>(*begin));
          if(colon.value().data == ":")
          {
            if(++begin == end)
            {
              throw expect::error::type::invalid_generic
              { "no type list after colon" };
            }
            return std::make_tuple
            (
              std::experimental::optional<parse::cell::list>
              { parse::expect::type<parse::cell::type::list>(*begin) },
              std::next(begin)
            );
          }

          return std::make_tuple
          (
            std::experimental::optional<parse::cell::list>{},
            std::next(begin)
          );
        }
      }
    }
  }
}
