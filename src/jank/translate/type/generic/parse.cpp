#include <algorithm>

#include <jank/parse/expect/type.hpp>
#include <jank/translate/type/generic/extract.hpp>
#include <jank/translate/type/generic/parse.hpp>
#include <jank/translate/type/generic/verify.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/builtin/type/normalize.hpp>
#include <jank/translate/expect/error/type/invalid_generic.hpp>

namespace jank
{
  namespace translate
  {
    namespace type
    {
      namespace generic
      {
        genericity<cell::detail::type_definition<cell::cell>> parse
        (
          parse::cell::list const &l,
          std::shared_ptr<environment::scope> const &scope
        )
        {
          genericity<cell::detail::type_definition<cell::cell>> ret;

          for(auto it(l.data.begin()); it != l.data.end(); ++it)
          {
            if
            (
              auto const &list = parse::expect::optional_cast
              <parse::cell::type::list>(*it)
            )
            {
              auto const parsed(parse(list.value(), scope));
              tuple<cell::detail::type_definition<cell::cell>> tup{};
              std::transform
              (
                parsed.parameters.begin(), parsed.parameters.end(),
                std::back_inserter(tup.data),
                [](auto const &p)
                {
                  return boost::get
                  <single<cell::detail::type_definition<cell::cell>>>(p).data;
                }
              );
              ret.parameters.push_back(tup);
              continue;
            }

            auto const &type_name
            (parse::expect::type<parse::cell::type::ident>(*it).data);
            auto const &type_def(scope->find_type(type_name));

            if(!type_def)
            {
              throw expect::error::type::exception<>
              { "unknown type " + type_name };
            }

            auto type
            (
              environment::builtin::type::normalize
              (type_def.value().first.data, *scope)
            );
            std::tie(type, it) = apply_genericity
            (std::move(type), it, l.data.end(), scope);

            ret.parameters.push_back
            (
              single<cell::detail::type_definition<cell::cell>>
              { type }
            );
          }

          return ret;
        }

        std::tuple
        <
          cell::detail::type_definition<cell::cell>,
          parse::cell::list::type::const_iterator
        > apply_genericity
        (
          cell::detail::type_definition<cell::cell> &&type,
          parse::cell::list::type::const_iterator const begin,
          parse::cell::list::type::const_iterator const end,
          std::shared_ptr<environment::scope> const &scope
        )
        {
          auto const &extracted_generic
          (type::generic::extract(begin, end));
          auto const &generic_list_opt(std::get<0>(extracted_generic));
          if(generic_list_opt)
          {
            auto const &parsed_generics
            (type::generic::parse(generic_list_opt.value(), scope));
            type::generic::verify
            (
              type.generics,
              parsed_generics
            );

            type.generics = parsed_generics;
          }
          return std::make_tuple(type, std::get<1>(extracted_generic));
        }
      }
    }
  }
}
