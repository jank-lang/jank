#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/error/lookup/exception.hpp>

namespace jank
{
  namespace interpret
  {
    namespace environment
    {
      cell::cell resolve_value
      (
        std::shared_ptr<scope> const &s,
        translate::cell::cell const &c
      )
      {
        switch(translate::cell::trait::to_enum(c))
        {
          case translate::cell::type::binding_reference:
          {
            auto const &cell
            (
              translate::expect::type
              <translate::cell::type::binding_reference>(c)
            );
            auto const opt(s->find_binding(cell.data.definition.name));
            if(!opt)
            {
              throw expect::error::lookup::exception<>
              { "unknown binding: " + cell.data.definition.name };
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
                return
                {
                  cell::null
                  { boost::get<parse::cell::null>(cell.data).data }
                };
              case translate::cell::literal_type::boolean:
                return
                {
                  cell::boolean
                  { boost::get<parse::cell::boolean>(cell.data).data }
                };
              case translate::cell::literal_type::integer:
                return
                {
                  cell::integer
                  { boost::get<parse::cell::integer>(cell.data).data }
                };
              case translate::cell::literal_type::real:
                return
                {
                  cell::real
                  { boost::get<parse::cell::real>(cell.data).data }
                };
              case translate::cell::literal_type::string:
                return
                {
                  cell::string
                  { boost::get<parse::cell::string>(cell.data).data }
                };
              default:
                throw expect::error::lookup::exception<>{ "invalid literal" };
            }
          } break;

          case translate::cell::type::function_call:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::function_call>(c));

            /* TODO: This is copied from interpret.cpp. */
            auto const next_scope(std::make_shared<scope>());
            next_scope->parent = s;

            auto arg_name_it(cell.data.definition.arguments.begin());
            for(auto const &arg : cell.data.arguments)
            {
              auto const &name(*arg_name_it++);
              auto const var(environment::resolve_value(next_scope, arg.cell));
              next_scope->bindings[name.name] = var;
            }

            /* TODO: This is nasty. */
            /* We need to look up the function body here, instead of using
             * the one we have; recursive calls will have empty bodies. */
            return interpret
            (
              next_scope,
              { cell.data.scope->expect_function(cell.data.definition) }
            );
          } break;

          case translate::cell::type::native_function_call:
          {
            auto const &cell
            (
              translate::expect::type
              <translate::cell::type::native_function_call>(c)
            );

            /* Recurse. */
            return environment::resolve_value
            (s, cell.data.definition.interpret(s, cell.data.arguments));
          } break;

          case translate::cell::type::function_body:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::function_body>(c));

            auto const next_scope(std::make_shared<scope>());
            next_scope->parent = s;

            return interpret(next_scope, cell);
          } break;

          case translate::cell::type::function_definition:
          {
            /* TODO: Handle properly. */
            std::cout << "resolving function definition" << std::endl;
            return cell::null{};
          } break;

          case translate::cell::type::function_reference:
          {
            /* TODO: Handle properly. */
            //auto const &cell
            //(
            //  translate::expect::type
            //  <translate::cell::type::function_reference>(c)
            //);
            std::cout << "resolving function reference" << std::endl;

            /* TODO: Loop up the function by name and type. */
            return cell::null{};
          } break;

          case translate::cell::type::native_function_reference:
          {
            /* TODO: Handle properly. */
            return cell::null{};
          } break;

          default:
            throw expect::error::lookup::exception<>
            { "invalid value: " + std::to_string(c.which()) };
        }
      }
    }
  }
}
