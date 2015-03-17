#pragma once

#include <jank/interpret/cell/cell.hpp>
#include <jank/interpret/expect/type.hpp>
#include <jank/interpret/expect/error/type/type.hpp>
#include <jank/interpret/environment/state.hpp>

namespace jank
{
  namespace interpret
  {
    cell::cell interpret(environment::state &env, cell::list &root)
    {
      auto const &func_name(expect::type<cell::type::ident>(root.data[0]).data);

      auto const special_it(env.find_special(func_name));
      if(special_it)
      { return special_it->data(env, root); }

      /* Collapse all nested lists to values. */
      for(auto &c : root.data)
      {
        /* TODO: (quote foo bar spam) */
        switch(static_cast<cell::type>(c.which()))
        {
          case cell::type::list:
            c = interpret(env, boost::get<cell::list>(c));
            break;
          case cell::type::ident:
          {
            auto const ident_it(env.find_cell(boost::get<cell::ident>(c).data));
            if(ident_it)
            { c = *ident_it; }
          } break;
          default:
            break;
        }
      }

      auto const env_it(env.find_function(func_name));
      if(!env_it)
      { throw expect::error::type::type<>{ "unknown function: " + func_name }; }

      auto const &func(env_it->data);
      return func(env, root);
    }
  }
}
