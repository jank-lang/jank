#include <jank/parse/cell/stream.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/translate/cell/stream.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/error/lookup/exception.hpp>
#include <jank/interpret/detail/function_call.hpp>
#include <jank/interpret/detail/native_function_call.hpp>
#include <jank/interpret/detail/variable_definition.hpp>

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
      /* TODO: Move each of these to their own source files. */
      for(auto const &c : root.data.cells)
      {
        switch(static_cast<translate::cell::type>(c.which()))
        {
          case translate::cell::type::function_call:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::function_call>(c));
            detail::function_call(env, cell);
          } break;

          case translate::cell::type::native_function_call:
          {
            auto const &cell
            (
              translate::expect::type
              <translate::cell::type::native_function_call>(c)
            );
            detail::native_function_call(env, cell);
          } break;

          case translate::cell::type::return_statement:
          {
            auto const &cell
            (
              translate::expect::type
              <translate::cell::type::return_statement>(c)
            );
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
            (
              translate::expect::type
              <translate::cell::type::variable_definition>(c)
            );
            detail::variable_definition(env, cell);
          } break;

          default:
            break;
        }
      }

      return parse::cell::null{};
    }
  }
}
