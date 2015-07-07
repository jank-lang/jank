#include <stdexcept>
#include <memory>

#include <jank/translate/environment/special/function.hpp>

#include <jank/parse/expect/type.hpp>
#include <jank/translate/translate.hpp>
#include <jank/translate/function/argument/definition.hpp>
#include <jank/translate/function/return/parse.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/expect/error/syntax/exception.hpp>
#include <jank/translate/expect/error/type/overload.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        cell::cell function
        (parse::cell::list const &input, cell::function_body const &outer_body)
        {
          static std::size_t constexpr forms_required{ 4 };

          auto &data(input.data);
          if(data.size() < forms_required)
          { throw expect::error::syntax::exception<>{ "invalid function definition" }; }

          auto const name(parse::expect::type<parse::cell::type::ident>(data[1]));
          auto const args(parse::expect::type<parse::cell::type::list>(data[2]));
          auto const nested_scope(std::make_shared<scope>(outer_body.data.scope));
          auto const arg_definitions
          (
            function::argument::definition::parse_types(args, nested_scope)
          );

          /* Add args to function's scope. */
          std::transform
          (
            arg_definitions.begin(), arg_definitions.end(),
            std::inserter
            (
              nested_scope->variable_definitions,
              nested_scope->variable_definitions.end()
            ),
            [](auto const &arg)
            {
              return std::make_pair
              (
                arg.name,
                cell::variable_definition{ { arg.name, arg.type, cell::detail::constness::non_constant } }
              );
            }
          );

          /* TODO: Check native functions, too. */
          /* Check for an already-defined function of this type. */
          /* XXX: We're only checking *this* scope's functions, so shadowing is allowed. */
          for(auto const &overload : outer_body.data.scope->function_definitions[name.data])
          {
            if(overload.data.arguments == arg_definitions)
            { throw expect::error::type::overload{ "multiple definition of: " + name.data }; }
          }

          /* TODO: Add multiple return types into a tuple. */
          /* Parse return types. */
          auto const return_type_names(parse::expect::type<parse::cell::type::list>(data[3]));
          auto const return_types(function::ret::parse(return_type_names, nested_scope));

          /* Add an empty declaration first, to allow for recursive references. */
          outer_body.data.scope->function_definitions[name.data].emplace_back();

          cell::function_definition const ret
          {
            {
              name.data,
              arg_definitions,
              return_types[0].data,
              translate /* Recurse into translate for the body. */
              (
                jtl::it::make_range(std::next(data.begin(), forms_required), data.end()),
                nested_scope,
                return_types[0]
              ).data,
              nested_scope
            }
          };

          /* Add the function definition to the out body's scope. */
          outer_body.data.scope->function_definitions[name.data].back() = ret;
          return { ret };
        }
      }
    }
  }
}
