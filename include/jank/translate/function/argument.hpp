#pragma once

#include <string>
#include <vector>
#include <ostream>

#include <jank/parse/cell/cell.hpp>
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
            { os << a.name << " : " << parse::cell::type_string(a.type) << " "; }
            os << ") ";
            return os;
          }
        }
        using list = std::vector<detail::argument>;

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
            ret.push_back({ name, parse::cell::type_from_string(type) });
          }

          return ret;
        }
      }
    }
  }
}
