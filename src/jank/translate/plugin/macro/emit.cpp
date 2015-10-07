#include <jank/translate/plugin/macro/emit.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/macro_primitive.hpp>
#include <jank/translate/environment/builtin/value/primitive.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace macro
      {
        static void make_emit
        (
          std::shared_ptr<environment::scope> const &scope,
          cell::detail::type_reference<cell::cell> const &type
        )
        {
          auto const nested_scope(std::make_shared<environment::scope>());
          nested_scope->parent = scope;

          cell::native_function_declaration def
          {
            {
              "emit",
              { { "data", type } },
              environment::builtin::type::null(*scope),
              nested_scope
            }
          };

          scope->native_function_declarations[def.data.name].emplace_back
          (std::move(def));
        }

        void emit(std::shared_ptr<environment::scope> const &scope)
        {
          /* TODO: If these are in the opposite order, shit doesn't work. */
          make_emit(scope, environment::builtin::type::macro_atom(*scope));
          make_emit(scope, environment::builtin::type::macro_list(*scope));
        }
      }
    }
  }
}
