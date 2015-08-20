#include <jank/parse/expect/type.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/detail/function_call.hpp>
#include <jank/interpret/detail/native_function_call.hpp>
#include <jank/interpret/detail/indirect_function_call.hpp>
#include <jank/interpret/detail/binding_definition.hpp>
#include <jank/interpret/detail/if_statement.hpp>
#include <jank/interpret/detail/return_statement.hpp>
#include <jank/interpret/detail/do_statement.hpp>

/* TODO: print with no newline. */
namespace jank
{
  namespace interpret
  {
    parse::cell::cell interpret
    (
      std::shared_ptr<environment::scope> const &scope,
      translate::cell::function_body const &root,
      consume_style const consume
    )
    {
      for(auto const &c : root.data.cells)
      {
        switch(translate::cell::trait::to_enum(c))
        {
          case translate::cell::type::function_call:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::function_call>(c));
            auto const &ret(detail::function_call(scope, cell));
            if(consume == consume_style::greedy)
            { return ret; }
          } break;

          case translate::cell::type::native_function_call:
          {
            auto const &cell
            (
              translate::expect::type
              <translate::cell::type::native_function_call>(c)
            );
            auto const &ret(detail::native_function_call(scope, cell));
            if(consume == consume_style::greedy)
            { return ret; }
          } break;

          case translate::cell::type::indirect_function_call:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::indirect_function_call>(c));
            auto const &ret(detail::indirect_function_call(scope, cell));
            if(consume == consume_style::greedy)
            { return ret; }
          } break;

          case translate::cell::type::return_statement:
          {
            auto const &cell
            (
              translate::expect::type
              <translate::cell::type::return_statement>(c)
            );
            return detail::return_statement(scope, cell);
          } break;

          case translate::cell::type::if_statement:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::if_statement>(c));
            auto const ret(detail::if_statement(scope, cell));
            if(!parse::expect::is<parse::cell::type::null>(ret))
            { return ret; }
          } break;

          case translate::cell::type::do_statement:
          {
            auto const &cell
            (translate::expect::type<translate::cell::type::do_statement>(c));
            auto const ret(detail::do_statement(scope, cell));
            if(!parse::expect::is<parse::cell::type::null>(ret))
            { return ret; }
          } break;

          /* Handles const and non-const. */
          case translate::cell::type::binding_definition:
          {
            auto const &cell
            (
              translate::expect::type
              <translate::cell::type::binding_definition>(c)
            );
            detail::binding_definition(scope, cell);
          } break;

          default:
            break;
        }
      }

      return parse::cell::null{};
    }

    parse::cell::cell interpret
    (
      std::shared_ptr<environment::scope> const &scope,
      translate::cell::function_body const &root
    )
    { return interpret(scope, root, consume_style::normal); }
  }
}
