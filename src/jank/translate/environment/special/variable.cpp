#include <jank/translate/environment/special/variable.hpp>

#include <jank/parse/expect/type.hpp>
#include <jank/translate/translate.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/function/argument/resolve_type.hpp>
#include <jank/translate/expect/error/syntax/syntax.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        namespace detail
        {
          namespace variable
          {
            cell::cell make
            (
              parse::cell::list const &input,
              cell::function_body const &body,
              cell::detail::constness c
            )
            {
              static std::size_t constexpr forms_required{ 4 };

              auto &data(input.data);
              if(data.size() < forms_required)
              { throw expect::error::syntax::exception<>{ "invalid variable definition" }; }

              auto const name(parse::expect::type<parse::cell::type::ident>(data[1]));
              auto const type_name(parse::expect::type<parse::cell::type::ident>(data[2]));

              auto const var_opt(body.data.scope->find_variable(name.data));
              if(var_opt && var_opt.value().second == body.data.scope)
              { throw expect::error::type::exception<>{ "multiple definition of: " + name.data }; }

              auto const type_opt(body.data.scope->find_type(type_name.data));
              if(!type_opt)
              { throw expect::error::type::exception<>{ "unknown type: " + type_name.data }; }
              auto const &type(type_opt.value().first);

              /* Remove everything but the type and the value and parse it as a function call.
               * The type will be skipped, as it's considered the function's name. */
              auto parsable_list(input);
              parsable_list.data.erase
              (
                parsable_list.data.begin(),
                std::next(parsable_list.data.begin(), 2)
              );
              auto const arguments
              (
                function::argument::call::parse<cell::cell>
                (parsable_list, body.data.scope)
              );
              if(arguments.empty())
              {
                throw expect::error::syntax::exception<>
                { "no value specified in variable definition" };
              }
              else if(arguments.size() > 1)
              {
                throw expect::error::syntax::exception<>
                { "multiple values specified in variable definition" };
              }

              auto const value_type(function::argument::resolve_type(arguments[0].cell, body.data.scope));
              if(value_type.data != type.data)
              { throw expect::error::type::exception<>{ "incompatible types for variable definition" }; }

              auto const def(cell::variable_definition{ { name.data, { value_type.data }, c } });
              body.data.scope->variable_definitions[name.data] = def;
              return { def };
            }
          }
        }

        cell::cell variable
        (parse::cell::list const &input, cell::function_body const &body)
        { return detail::variable::make(input, body, cell::detail::constness::non_constant); }
      }
    }
  }
}
