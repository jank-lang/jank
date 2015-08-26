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
          parse::cell::list::type::const_iterator const begin,
          parse::cell::list::type::const_iterator const end
        )
        {
          auto const colon_it(std::next(begin));
          auto const &colon
          (
            parse::expect::optional_cast<parse::cell::type::ident>
            (*colon_it)
          );
          if(colon.value().data == ":")
          {
            auto const list_it(std::next(colon_it));
            if(list_it == end)
            {
              throw expect::error::type::invalid_generic
              { "no type list after colon" };
            }
            return std::make_tuple
            (
              std::experimental::optional<parse::cell::list>
              { parse::expect::type<parse::cell::type::list>(*list_it) },
              list_it
            );
          }

          return std::make_tuple
          (
            std::experimental::optional<parse::cell::list>{},
            begin
          );
        }
      }
    }
  }
}
