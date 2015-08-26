#pragma once

#include <jank/parse/cell/cell.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/translate/cell/detail/type_definition.hpp>
#include <jank/translate/type/generic/genericity.hpp>
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
        inline genericity<cell::detail::type_definition> parse /* TODO: move to cpp */
        (
          parse::cell::list const &l,
          std::shared_ptr<environment::scope> const &scope
        )
        {
          genericity<cell::detail::type_definition> ret;

          for(auto it(l.data.begin()); it != l.data.end(); ++it)
          {
            /* TODO: Handle lists/tuples by recursing. */
            auto const &type
            (parse::expect::type<parse::cell::type::ident>(*it).data);
            auto const &type_def(scope->find_type(type));
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
