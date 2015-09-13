#include <algorithm>

#include <jank/parse/cell/trait.hpp>
#include <jank/translate/cell/trait.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/function.hpp>
#include <jank/translate/function/argument/resolve_type.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/translate/expect/error/type/exception.hpp>
#include <jank/translate/expect/error/internal/exception.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace argument
      {
        cell::type_definition resolve_type
        (
          cell::cell const &c,
          std::shared_ptr<environment::scope> const &scope
        )
        {
          switch(cell::trait::to_enum(c))
          {
            case cell::type::binding_reference:
            {
              auto const &ref(expect::type<cell::type::binding_reference>(c));
              return { ref.data.definition.type.definition };
            }
            case cell::type::literal_value:
            {
              auto const &literal(expect::type<cell::type::literal_value>(c));
              auto const name
              (
                parse::cell::trait::to_string
                (parse::cell::trait::to_enum(literal.data))
              );

              auto const &def_opt(scope->find_type(name));
              if(!def_opt)
              {
                throw expect::error::type::exception<>
                { std::string{ "invalid literal type: " } + name };
              }

              return def_opt.value().first;
            }
            case cell::type::function_call:
            {
              auto const &call(expect::type<cell::type::function_call>(c));
              return { call.data.definition.return_type.definition };
            }
            case cell::type::native_function_call:
            {
              auto const &call(expect::type<cell::type::native_function_call>(c));
              return { call.data.definition.return_type.definition };
            }
            case cell::type::indirect_function_call:
            {
              auto const &call(expect::type<cell::type::indirect_function_call>(c));
              auto const &generics(call.data.binding.type.definition.generics);
              if(generics.parameters.size() != 2)
              {
                throw expect::error::internal::exception<>
                { "invalid indirect function call" };
              }

              /* Empty means a null return. */
              auto const &ret_tuple
              (
                boost::get
                <type::generic::tuple<cell::detail::type_definition<cell::cell>>>
                (call.data.binding.type.definition.generics.parameters[1])
              );
              if(ret_tuple.data.empty())
              { return { environment::builtin::type::null(*scope).definition }; }

              /* TODO: Handle multiple return types. */
              return { ret_tuple.data.front() };
            }
            case cell::type::function_body:
            {
              auto const &body(expect::type<cell::type::function_body>(c));
              return { body.data.return_type.definition };
            }
            case cell::type::function_definition:
            {
              auto const &body(expect::type<cell::type::function_definition>(c));
              auto def(environment::builtin::type::function(*scope).definition);

              type::generic::tuple
              <cell::detail::type_definition<cell::cell>> args;
              std::transform
              (
                body.data.arguments.begin(), body.data.arguments.end(),
                std::back_inserter(args.data),
                [](auto const &arg)
                { return arg.type.definition; }
              );

              /* Null return types should be considered empty lists. */
              type::generic::tuple
              <cell::detail::type_definition<cell::cell>> returns;
              if
              (
                body.data.return_type.definition !=
                environment::builtin::type::null(*scope).definition
              )
              { returns.data.push_back(body.data.return_type.definition); }

              def.generics.parameters.clear();
              def.generics.parameters.push_back(args);
              def.generics.parameters.push_back(returns);

              return { def };
            }
            case cell::type::function_reference:
            {
              /* Recurse with the definition. */
              auto const &body(expect::type<cell::type::function_reference>(c));
              return resolve_type
              (
                cell::function_definition{ body.data.definition },
                scope
              );
            }
            default:
              throw expect::error::type::exception<>
              { "invalid argument type: " + std::to_string(c.which()) };
          }
        }
      }
    }
  }
}
