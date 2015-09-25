#include <jank/parse/expect/type.hpp>
#include <jank/translate/type/generic/extract.hpp>
#include <jank/translate/expect/error/type/invalid_generic.hpp>

namespace jank
{
  namespace translate
  {
    namespace type
    {
      namespace generic
      {
        std::tuple
        <
          boost::optional<parse::cell::list>,
          parse::cell::list::type::const_iterator
        > extract
        (
          parse::cell::list::type::const_iterator const begin,
          parse::cell::list::type::const_iterator const end
        )
        {
          auto const colon_it(std::next(begin));
          if(colon_it == end)
          {
            return std::make_tuple
            (
              boost::optional<parse::cell::list>{},
              begin
            );
          }

          auto const &colon
          (
            parse::expect::optional_cast<parse::cell::type::ident>
            (*colon_it)
          );
          if(colon && colon->data == ":")
          {
            auto const list_it(std::next(colon_it));
            if(list_it == end)
            {
              throw expect::error::type::invalid_generic
              { "no type list after colon" };
            }
            return std::make_tuple
            (
              boost::optional<parse::cell::list>
              { parse::expect::type<parse::cell::type::list>(*list_it) },
              list_it
            );
          }

          return std::make_tuple
          (
            boost::optional<parse::cell::list>{},
            begin
          );
        }
      }
    }
  }
}
