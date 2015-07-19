#include <jank/translate/expect/type.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/error/lookup/exception.hpp>

namespace jank
{
  namespace interpret
  {
    /* TODO: This should be in the environment namespace. */
    parse::cell::cell resolve_value
    (
      std::shared_ptr<environment::scope> const &scope,
      translate::cell::cell const &c
    )
    {
      switch(static_cast<translate::cell::type>(c.which()))
      {
        case translate::cell::type::variable_reference:
        {
          auto const &cell
          (
            translate::expect::type
            <translate::cell::type::variable_reference>(c)
          );
          auto const opt(scope->find_variable(cell.data.definition.name));
          if(!opt)
          {
            throw expect::error::lookup::exception<>
            { "unknown variable: " + cell.data.definition.name };
          }
          return opt.value();
        } break;

        case translate::cell::type::literal_value:
        {
          auto const &cell
          (translate::expect::type<translate::cell::type::literal_value>(c));
          switch(static_cast<translate::cell::literal_type>(cell.data.which()))
          {
            case translate::cell::literal_type::null:
              return boost::get<parse::cell::null>(cell.data);
            case translate::cell::literal_type::boolean:
              return boost::get<parse::cell::boolean>(cell.data);
            case translate::cell::literal_type::integer:
              return boost::get<parse::cell::integer>(cell.data);
            case translate::cell::literal_type::real:
              return boost::get<parse::cell::real>(cell.data);
            case translate::cell::literal_type::string:
              return boost::get<parse::cell::string>(cell.data);
            default:
              throw expect::error::lookup::exception<>{ "invalid literal" };
          }
        } break;

        case translate::cell::type::function_call:
        {
          auto const &cell
          (translate::expect::type<translate::cell::type::function_call>(c));

          /* TODO: This is copied from interpret.cpp. */
          auto const next_scope(std::make_shared<environment::scope>());
          next_scope->parent = scope;

          auto arg_name_it(cell.data.definition.arguments.begin());
          for(auto const &arg : cell.data.arguments)
          {
            auto const &name(*arg_name_it++);
            auto const var(resolve_value(next_scope, arg.cell));
            next_scope->variables[name.name] = var;
          }

          return interpret(next_scope, { cell.data.definition.body });
        } break;

        case translate::cell::type::native_function_call:
        {
          auto const &cell
          (
            translate::expect::type
            <translate::cell::type::native_function_call>(c)
          );

          /* Recurse. */
          return resolve_value
          (scope, cell.data.definition.interpret(scope, cell.data.arguments));
        } break;

        default:
          throw expect::error::lookup::exception<>
          { "invalid value: " + std::to_string(c.which()) };
      }
    }
  }
}
