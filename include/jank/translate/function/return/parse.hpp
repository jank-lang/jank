#pragma once

#include <vector>

#include <jank/parse/cell/trait.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/expect/error/internal/unimplemented.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace ret
      {
        using type_list = std::vector<cell::type_reference>;

        /* TODO: refactor into cpp */
        inline type_list parse
        (
          parse::cell::list const &list,
          std::shared_ptr<environment::scope> const &scope
        )
        {
          if(list.data.size() > 1)
          { throw expect::error::internal::unimplemented{ "multiple return types" }; }
          else if(list.data.empty()) /* No return type means null return type. */
          {
            auto const null
            (
              scope->find_type
              (parse::cell::trait::to_string<parse::cell::type::null>())
            );
            if(!null) /* Shouldn't ever happen. */
            { throw expect::error::internal::exception<>{ "no null type found" }; }
            return { { { null.value().data } } };
          }

          /* Resolve each type. */
          type_list types;
          for(auto const &t : list.data)
          {
            auto const &type_string
            (parse::expect::type<parse::cell::type::ident>(t).data);
            auto const type(scope->find_type(type_string));
            if(!type)
            {
              throw expect::error::type::exception<>
              { "invalid return type: " + type_string };
            }
            types.push_back({ { type.value().data } });
          }

          return types;
        }
      }
    }
  }
}
