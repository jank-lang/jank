#include <vector>

#include <jank/translate/function/return/parse.hpp>
#include <jank/parse/cell/trait.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/translate/type/generic/extract.hpp>
#include <jank/translate/type/generic/parse.hpp>
#include <jank/translate/type/generic/verify.hpp>
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
          /* No return type means null return type. */
          if(list.data.empty())
          { return { { { environment::builtin::type::null(*scope) } } }; }

          /* Resolve each type. */
          type_list types;
          for(auto it(list.data.begin()); it != list.data.end(); ++it)
          {
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
            std::tie(type, it) = type::generic::apply_genericity
            (std::move(type), it, list.data.end(), scope);
            types.push_back({ { type } });
          }

          if(types.size() > 1)
          {
            throw expect::error::internal::unimplemented
            { "multiple return types" };
          }

          return types;
        }
      }
    }
  }
}
