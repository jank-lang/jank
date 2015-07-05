#include <jank/translate/plugin/io/print.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace io
      {
        void print(std::shared_ptr<environment::scope> const &scope)
        {
          auto const nested_scope(std::make_shared<environment::scope>());
          nested_scope->parent = scope;

          cell::native_function_definition def
          {
            {
              "print",
              { { "i", environment::builtin::type::integer(scope) } },
              environment::builtin::type::null(scope),
              [](auto const &) -> function::argument::detail::value<cell::cell>
              {
                //return environment::builtin::type::null(scope);
                return {};
              },
              nested_scope
            }
          };
          scope->native_function_definitions[def.data.name].emplace_back
          (std::move(def));
        }
      }
    }
  }
}
