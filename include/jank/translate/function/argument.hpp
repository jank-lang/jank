#pragma once

#include <string>
#include <vector>
#include <ostream>

#include <jank/parse/cell/cell.hpp>
#include <jank/parse/cell/visit.hpp>
#include <jank/parse/cell/trait.hpp>
#include <jank/translate/cell/type.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/translate/expect/error/syntax/syntax.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace argument
      {
        namespace detail
        {
          using list = std::vector<struct argument>;
          struct argument
          {
            std::string name;
            parse::cell::type type;
          };

          inline bool operator ==(list const &lhs, list const &rhs)
          {
            return (lhs.size() == rhs.size()) &&
                    std::equal(lhs.begin(), lhs.end(), rhs.begin(),
                               [](auto const &lhs, auto const &rhs)
                               { return lhs.type == rhs.type; });
          }

          inline std::ostream& operator <<(std::ostream &os, list const &args)
          {
            os << "( ";
            for(auto const &a : args)
            { os << a.name << " : " << parse::cell::trait::enum_to_string(a.type) << " "; }
            os << ") ";
            return os;
          }
        }
        using list = std::vector<detail::argument>;

        namespace definition
        {
          inline list parse(parse::cell::list const &l)
          {
            list ret;

            for(auto it(l.data.begin()); it != l.data.end(); ++it)
            {
              auto const &name(expect::type<parse::cell::type::ident>(*it).data);
              if(++it == l.data.end())
              {
                throw expect::error::syntax::syntax<>
                { "syntax error: expected type after " + name };
              }

              auto const &type(expect::type<parse::cell::type::ident>(*it).data);
              ret.push_back({ name, parse::cell::trait::enum_from_string(type) });
            }

            return ret;
          }
        }

        namespace call
        {
          template <typename Scope>
          list parse
          (
            parse::cell::list const &l,
            Scope const &/*scope*/
          )
          {
            list ret;

            std::transform
            (
              l.data.begin(), l.data.end(), std::back_inserter(ret),
              [](auto const &a)
              {
                /* TODO: Read type from scope. */
                //return detail::argument{ "", parse::cell::type::integer };
                return parse::cell::visit
                (
                  a, [](auto const &c)
                  {
                    if(std::is_same<decltype(c), parse::cell::ident>::value)
                    { throw expect::error::type::type<>{ "unsupported ident in function call" }; }
                    else
                    {
                      return detail::argument
                      {
                        std::string{ "rvalue " } + parse::cell::trait::enum_to_string<decltype(c)::value>(),
                        decltype(c)::value
                      };
                    }
                  }
                );
              }
            );

            return ret;
          }
        }
      }
    }
  }
}
