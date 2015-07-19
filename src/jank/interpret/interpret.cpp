#include <jank/parse/cell/stream.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/translate/cell/stream.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/error/lookup/exception.hpp>

/* TODO: print with no newline. */
namespace jank
{
  namespace interpret
  {
    parse::cell::cell interpret
    (
      std::shared_ptr<environment::scope> const &env,
      translate::cell::function_body const &root
    )
    {
      for(auto const &c : root.data.cells)
      {
        switch(static_cast<translate::cell::type>(c.which()))
        {
          case translate::cell::type::function_call:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::function_call>(c));

            auto const next_scope(std::make_shared<environment::scope>());
            next_scope->parent = env;

            auto arg_name_it(cell.data.definition.arguments.begin());
            for(auto const &arg : cell.data.arguments)
            {
              auto const &name(*arg_name_it++);
              auto const var(resolve_value(next_scope, arg.cell));
              next_scope->variables[name.name] = var;
            }

            interpret(next_scope, { cell.data.definition.body });
          } break;

          case translate::cell::type::native_function_call:
          {
            auto const &cell
            (
              translate::expect::type
              <translate::cell::type::native_function_call>(c)
            );

            resolve_value
            (env, cell.data.definition.interpret(env, cell.data.arguments));
          } break;

          case translate::cell::type::return_statement:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::return_statement>(c));
            return resolve_value(env, cell.data.cell);
          } break;

          case translate::cell::type::if_statement:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::if_statement>(c));
            auto const condition
            (resolve_value(env, cell.data.condition));
            if(parse::expect::type<parse::cell::type::boolean>(condition).data)
            { interpret(env, { cell.data.true_body }); }
            else
            { interpret(env, { cell.data.false_body }); }
          } break;

          case translate::cell::type::do_statement:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::do_statement>(c));
            interpret(env, { cell.data.body });
          } break;

          /* Handles const and non-const. */
          case translate::cell::type::variable_definition:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::variable_definition>(c));

            env->variables[cell.data.name] = resolve_value
            (
              env,
              cell.data.cell
            );
          } break;

          default:
            break;
        }
      }

      return parse::cell::null{};
    }
  }
}
