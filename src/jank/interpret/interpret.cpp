#include <jank/translate/expect/type.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/expect/type.hpp>
#include <jank/interpret/expect/error/type/type.hpp>

namespace jank
{
  namespace interpret
  {
    parse::cell::cell interpret(std::shared_ptr<scope> const &env, translate::cell::function_body const &root)
    {
      static_cast<void>(env);
      for(auto const &c : root.data.cells)
      {
        switch(static_cast<translate::cell::type>(c.which()))
        {
          case translate::cell::type::function_call:
          {
            auto const &cell(translate::expect::type<translate::cell::type::function_call>(c));
            if(cell.data.definition.name == "print")
            {
              std::cout << "print call" << std::endl;
            }

          } break;

          case translate::cell::type::function_body:
          case translate::cell::type::function_definition:
          case translate::cell::type::type_definition:
          case translate::cell::type::type_reference:
          case translate::cell::type::variable_definition:
          case translate::cell::type::variable_reference:
          case translate::cell::type::literal_value:
          case translate::cell::type::return_statement:
          default:
            break;
        }
      }

      return {};
    }

    parse::cell::cell interpret(environment::scope &env, parse::cell::list &root)
    {
      auto const &func_name(expect::type<parse::cell::type::ident>(root.data[0]).data);

      auto const special_it(env.find_special(func_name));
      if(special_it)
      { return special_it->data(env, root); }

      /* Collapse all nested lists to values. */
      for(auto &c : root.data)
      {
        /* TODO: (quote foo bar spam) */
        switch(static_cast<parse::cell::type>(c.which()))
        {
          case parse::cell::type::list:
            c = interpret(env, boost::get<parse::cell::list>(c));
            break;
          case parse::cell::type::ident:
          {
            auto const ident_it(env.find_cell(boost::get<parse::cell::ident>(c).data));
            if(ident_it)
            { c = *ident_it; }
          } break;
          default:
            break;
        }
      }

      auto const env_it(env.find_function(func_name));
      if(!env_it)
      { throw expect::error::type::exception<>{ "unknown function: " + func_name }; }

      auto const &func(env_it->data);
      return func(env, root);
    }
  }
}
