#include <jank/parse/cell/stream.hpp>
#include <jank/translate/cell/stream.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/expect/error/lookup/exception.hpp>

/* TODO: rename except files to exception.hpp */

namespace jank
{
  namespace interpret
  {
    /* TODO: cleanup */
    parse::cell::cell resolve_value(std::shared_ptr<scope> const &scope, translate::cell::cell const &c)
    {
      switch(static_cast<translate::cell::type>(c.which()))
      {
        case translate::cell::type::variable_reference:
        {
          auto const &cell(translate::expect::type<translate::cell::type::variable_reference>(c));
          auto const opt(scope->find_variable(cell.data.definition.name));
          if(!opt)
          { throw expect::error::lookup::exception<>{ "unknown variable: " + cell.data.definition.name }; }
          return opt.value();
        } break;

        case translate::cell::type::literal_value:
        {
          auto const &cell(translate::expect::type<translate::cell::type::literal_value>(c));
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
          auto const &cell(translate::expect::type<translate::cell::type::function_call>(c));
          return interpret(scope, { cell.data.definition.body });
        } break;

        default:
          throw expect::error::lookup::exception<>{ "invalid value: " + std::to_string(c.which()) };
      }
    }

    parse::cell::cell interpret
    (
      std::shared_ptr<scope> const &env,
      translate::cell::function_body const &root
    )
    {
      static_cast<void>(env);
      for(auto const &c : root.data.cells)
      {
        switch(static_cast<translate::cell::type>(c.which()))
        {
          case translate::cell::type::function_call:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::function_call>(c));

            auto const next_scope(std::make_shared<scope>());
            next_scope->parent = env;

            auto arg_name_it(cell.data.definition.arguments.begin());
            for(auto const &arg : cell.data.arguments)
            {
              auto const &name(*arg_name_it++);
              auto const var(resolve_value(next_scope, arg.cell));
              std::cout << "adding " << name.name
                        << " to next scope as: " << var << std::endl;
              next_scope->variables[name.name] = var;
            }

            std::cout << "call (" << cell.data.definition.name << "): ";
            for(auto const &arg : cell.data.arguments)
            { std::cout << resolve_value(env, arg.cell) << " "; }
            std::cout << std::endl;

            return interpret(next_scope, { cell.data.definition.body });
          } break;

          case translate::cell::type::return_statement:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::return_statement>(c));
            std::cout << "returning: " << cell.data.cell << std::endl;
            return resolve_value(env, cell.data.cell);
          } break;

          default:
            break;
        }
      }

      return parse::cell::null{};
    }
  }
}
