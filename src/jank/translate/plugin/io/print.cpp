#include <jank/translate/plugin/io/print.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/value/primitive.hpp>
#include <jank/translate/cell/stream.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/parse/cell/stream.hpp>

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

          /* TODO: Add overloads for all primitives. */
          cell::native_function_definition def
          {
            {
              "print",
              { { "i", environment::builtin::type::integer(*scope) } },
              environment::builtin::type::null(*scope),
              [](auto const &scope, auto const &args) -> cell::cell
              {
                for(auto const &arg : args)
                { std::cout << interpret::resolve_value(scope, arg.cell); }
                std::cout << std::endl;
                return environment::builtin::value::null();
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
