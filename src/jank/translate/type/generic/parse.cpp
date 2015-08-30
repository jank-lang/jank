#include <algorithm>

#include <jank/translate/type/generic/parse.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/expect/error/type/invalid_generic.hpp>

namespace jank
{
  namespace translate
  {
    namespace type
    {
      namespace generic
      {
        genericity<cell::detail::type_definition> parse
        (
          parse::cell::list const &l,
          std::shared_ptr<environment::scope> const &scope
        )
        {
          genericity<cell::detail::type_definition> ret;

          for(auto it(l.data.begin()); it != l.data.end(); ++it)
          {
            if
            (
              auto const &list = parse::expect::optional_cast
              <parse::cell::type::list>(*it)
            )
            {
              auto const parsed(parse(list.value(), scope));
              tuple<cell::detail::type_definition> tup{};
              std::transform
              (
                parsed.parameters.begin(), parsed.parameters.end(),
                std::back_inserter(tup.data),
                [](auto const &p)
                {
                  return boost::get
                  <single<cell::detail::type_definition>>(p).data;
                }
              );
              ret.parameters.push_back(tup);
              continue;
            }

            auto const &type
            (parse::expect::type<parse::cell::type::ident>(*it).data);
            auto const &type_def(scope->find_type(type));

            /* TODO: Try to extract generics from this type recursively. */
            if(!type_def)
            {
              throw expect::error::type::exception<>
              { "unknown type " + type };
            }
            ret.parameters.push_back
            (
              single<cell::detail::type_definition>
              { type_def.value().first.data }
            );
          }

          return ret;
        }
      }
    }
  }
}
