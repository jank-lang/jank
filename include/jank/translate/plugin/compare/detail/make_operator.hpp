#pragma once

#include <jank/parse/expect/type.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/value/primitive.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace compare
      {
        namespace detail
        {
          template <typename F>
          void make_operator
          (
            std::shared_ptr<environment::scope> const &scope,
            std::string const &op,
            cell::detail::type_reference const &type,
            F const &apply
          )
          {
            auto const nested_scope(std::make_shared<environment::scope>());
            nested_scope->parent = scope;

            cell::native_function_definition def
            {
              {
                op,
                { { "data1", type }, { "data2", type } },
                environment::builtin::type::boolean(*scope),
                [apply](auto const &scope, auto const &args) -> cell::cell
                {
                  if(args.size() != 2)
                  { throw expect::error::type::exception<>{ "invalid comparison operator args" }; }
                  return apply(scope, args);
                },
                nested_scope
              }
            };

            /* TODO: It's easy to overwrite something here; provide an API to register
             * native functions. */
            scope->native_function_definitions[def.data.name].emplace_back
            (std::move(def));
          }
        }
      }
    }
  }
}
