#include <string>
#include <vector>
#include <ostream>

#include <jank/translate/function/argument/definition.hpp>

#include <jank/parse/expect/type.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/translate/type/generic/extract.hpp>
#include <jank/translate/type/generic/parse.hpp>
#include <jank/translate/type/generic/verify.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/builtin/type/normalize.hpp>
#include <jank/translate/expect/error/syntax/exception.hpp>
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
            { os << a.name << " : " << a.type.definition.name << " "; }
            os << ") ";
            return os;
          }
        }

        namespace definition
        {
          type_list parse_types
          (
            parse::cell::list const &l,
            std::shared_ptr<environment::scope> const &scope
          )
          {
            type_list ret;

            for(auto it(l.data.begin()); it != l.data.end(); ++it)
            {
              auto const &name
              (parse::expect::type<parse::cell::type::ident>(*it).data);
              if(++it == l.data.end())
              {
                throw expect::error::syntax::exception<>
                { "expected type after " + name };
              }

              auto const &type_name
              (parse::expect::type<parse::cell::type::ident>(*it).data);
              auto const &type_def(scope->find_type(type_name));
              if(!type_def)
              {
                throw expect::error::type::exception<>
                { "unknown type " + type_name };
              }

              auto type(type_def.value().first);
              auto const &extracted_generic
              (type::generic::extract(it, l.data.end()));
              it = std::get<1>(extracted_generic);
              auto const &generic_list_opt(std::get<0>(extracted_generic));
              if(generic_list_opt)
              {
                auto const &parsed_generics
                (type::generic::parse(generic_list_opt.value(), scope));
                type::generic::verify
                (
                  type_def.value().first.data.generics,
                  parsed_generics
                );

                type.data.generics = parsed_generics;
              }

              ret.push_back
              (
                {
                  name,
                  {
                    environment::builtin::type::normalize
                    (type.data, *scope)
                  }
                }
              );
            }

            return ret;
          }
        }
      }
    }
  }
}
