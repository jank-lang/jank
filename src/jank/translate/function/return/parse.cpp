#include <vector>

#include <jank/translate/function/return/parse.hpp>
#include <jank/parse/cell/trait.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/normalize.hpp>
#include <jank/translate/expect/error/internal/unimplemented.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace ret
      {
        type_list parse
        (
          parse::cell::list const &list,
          std::shared_ptr<environment::scope> const &scope
        )
        {
          if(list.data.size() > 1)
          {
            throw expect::error::internal::unimplemented
            { "multiple return types" };
          }
          /* No return type means null return type. */
          else if(list.data.empty())
          { return { { { environment::builtin::type::null(*scope) } } }; }

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
            types.push_back
            (
              { {
                environment::builtin::type::normalize
                (type.value().first.data, *scope)
              } }
            );
          }

          return types;
        }
      }
    }
  }
}
