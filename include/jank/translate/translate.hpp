#pragma once

#include <jank/parse/cell/cell.hpp>

namespace jank
{
  namespace translate
  {
    parse::cell::cell translate(parse::cell::list &root)
    {
      //auto const &func_name(expect::type<parse::cell::type::ident>(root.data[0]).data);

      //auto const special_it(env.find_special(func_name));
      //if(special_it)
      //{ return special_it->data(env, root); }

      ///* Collapse all nested lists to values. */
      //for(auto &c : root.data)
      //{
      //  switch(static_cast<parse::cell::type>(c.which()))
      //  {
      //    case parse::cell::type::list:
      //      c = interpret(env, boost::get<parse::cell::list>(c));
      //      break;
      //    case parse::cell::type::ident:
      //    {
      //      auto const ident_it(env.find_cell(boost::get<parse::cell::ident>(c).data));
      //      if(ident_it)
      //      { c = *ident_it; }
      //    } break;
      //    default:
      //      break;
      //  }
      //}

      //auto const env_it(env.find_function(func_name));
      //if(!env_it)
      //{ throw expect::error::type::type<>{ "unknown function: " + func_name }; }

      //auto const &func(env_it->data);
      //return func(env, root);
    }
  }
}
