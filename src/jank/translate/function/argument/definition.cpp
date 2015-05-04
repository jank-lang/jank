#include <string>
#include <vector>
#include <ostream>

#include <jank/translate/function/argument/definition.hpp>

#include <jank/translate/cell/cell.hpp>
#include <jank/translate/expect/error/syntax/syntax.hpp>
#include <jank/translate/expect/error/internal/unimplemented.hpp>

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
          bool operator ==(type_list const &lhs, type_list const &rhs)
          {
            return (lhs.size() == rhs.size()) &&
                    std::equal(lhs.begin(), lhs.end(), rhs.begin(),
                               [](auto const &lhs, auto const &rhs)
                               { return lhs.type == rhs.type; });
          }

          std::ostream& operator <<(std::ostream &os, type_list const &args)
          {
            os << "( ";
            for(auto const &a : args)
            { os << a.name << " : " << parse::cell::trait::enum_to_string(a.type) << " "; }
            os << ") ";
            return os;
          }
        }

        namespace definition
        {
          type_list parse_types(parse::cell::list const &l)
          {
            type_list ret;

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
      }
    }
  }
}
