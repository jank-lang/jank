#include <jank/translate/environment/special/return_statement.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/function/argument/call.hpp>
#include <jank/translate/function/argument/resolve_type.hpp>
#include <jank/translate/expect/error/internal/unimplemented.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        cell::cell return_statement
        (parse::cell::list const &input, cell::function_body const &outer_body)
        {
          auto const args
          (
            function::argument::call::parse<cell::cell>
            (input, outer_body.data.scope)
          );

          if(args.size() > 1)
          { throw expect::error::internal::unimplemented{ "multiple return values" }; }
          else if(args.size() == 1)
          {
            auto const type
            (
              function::argument::resolve_type
              (args[0].cell, outer_body.data.scope)
            );
            if(type.data != outer_body.data.return_type.definition)
            { throw expect::error::type::exception<>{ "invalid return type" }; }

            return { cell::return_statement{ { args[0].cell, { type.data } } } };
          }

          /* Fallback to returning null. */
          cell::type_reference type
          { environment::builtin::type::null(outer_body.data.scope) };
          if(type.data != outer_body.data.return_type)
          { throw expect::error::type::exception<>{ "invalid return type" }; }
          return { cell::return_statement{ { {}, type.data } } };
        }
      }
    }
  }
}
