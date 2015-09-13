#include <algorithm>

#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/detail/function_call.hpp>
#include <jank/interpret/detail/indirect_function_call.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/error/lookup/exception.hpp>
#include <jank/interpret/expect/error/internal/unimplemented.hpp>

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
              case translate::cell::literal_type::list:
              {
                auto trans_list
                (boost::get<std::list<parse::cell::integer>>(cell.data));
                cell::list int_list;
                std::transform
                (
                  trans_list.begin(), trans_list.end(),
                  std::back_inserter(int_list.data),
                  [](auto const &trans_cell)
                  { return cell::integer{ trans_cell.data }; }
                );
                return int_list;
              }
              default:
                throw expect::error::lookup::exception<>{ "invalid literal" };
            }
          } break;

          case translate::cell::type::function_call:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::function_call>(c));

            /* We need to look up the function body here, instead of using
             * the one we have; recursive calls will have empty bodies. */
            return detail::function_call
            (
              s, cell,
              [&](auto const &scope, auto const &/*body*/)
              {
                return interpret
                (
                  scope,
                  { cell.data.scope->expect_function(cell.data.definition) }
                );
              }
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

          case translate::cell::type::indirect_function_call:
          {
            auto const &cell
            (
              translate::expect::type
              <translate::cell::type::indirect_function_call>(c)
            );

            return interpret::detail::indirect_function_call(s, cell);
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
            auto const &cell
            (
              translate::expect::type
              <translate::cell::type::function_definition>(c)
            );
            return cell::function{ cell.data };
          } break;

          case translate::cell::type::function_reference:
          {
            auto const &cell
            (
              translate::expect::type
              <translate::cell::type::function_reference>(c)
            );
            return cell::function{ cell.data.definition };
          } break;

          case translate::cell::type::native_function_reference:
          {
            /* TODO: Handle properly. */
            throw expect::error::internal::unimplemented
            { "native function references" };
          } break;

          default:
            throw expect::error::lookup::exception<>
            { "invalid value: " + std::to_string(c.which()) };
        }
      }
    }
  }
}
