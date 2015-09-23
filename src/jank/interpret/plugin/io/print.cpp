#include <jank/translate/environment/scope.hpp>
#include <jank/interpret/cell/stream.hpp>
#include <jank/interpret/plugin/io/print.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/error/internal/exception.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace io
      {
        void print
        (
          std::shared_ptr<translate::environment::scope> const &trans_scope,
          std::shared_ptr<environment::scope> const &int_scope
        )
        {
          auto const &defs(trans_scope->find_native_function("print"));
          if(!defs)
          {
            throw expect::error::internal::exception<>
            { "no native print declarations" };
          }

          for(auto const &def_pair : *defs)
          {
            auto const &def(def_pair.first.data);
            int_scope->native_function_definitions[def] =
            {
              def.name,
              [](auto const &scope, auto const &args) -> cell::cell
              {
                for(auto const &arg : args)
                { std::cout << interpret::environment::resolve_value(scope, arg.cell); }
                std::cout << std::endl;
                return cell::null{};
              }
            };
          }
        }
      }
    }
  }
}
