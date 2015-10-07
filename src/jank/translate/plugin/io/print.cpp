#include <jank/translate/plugin/io/print.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/list.hpp>
#include <jank/translate/environment/builtin/value/primitive.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace io
      {
        static void make_print
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
              "print",
              { { "data", type } },
              environment::builtin::type::null(*scope),
              nested_scope
            }
          };

          /* TODO: Use make_function. */
          scope->native_function_declarations[def.data.name].emplace_back
          (std::move(def));
        }

        void print(std::shared_ptr<environment::scope> const &scope)
        {
          make_print(scope, environment::builtin::type::null(*scope));
          make_print(scope, environment::builtin::type::boolean(*scope));
          make_print(scope, environment::builtin::type::integer(*scope));
          make_print(scope, environment::builtin::type::real(*scope));
          make_print(scope, environment::builtin::type::string(*scope));
          make_print
          (
            scope,
            environment::builtin::type::list
            (
              *scope,
              environment::builtin::type::integer(*scope)
            )
          );
        }
      }
    }
  }
}
