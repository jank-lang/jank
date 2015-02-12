#pragma once

#include "cell.hpp"

cell interpret(cell_list &root)
{
  auto const &func_name(boost::get<cell_ident>(root.data[0]).data);

  /* TODO: first handle special forms and shit. */
  auto const special_it(env.special_funcs.find(func_name));
  if(special_it != env.special_funcs.end())
  { return special_it->second.data(root); }

  /* Collapse all nested lists to values. */
  for(auto &c : root.data)
  {
    /* TODO: (quote foo bar spam) */
    switch(static_cast<cell_type>(c.which()))
    {
      case cell_type::list:
        c = interpret(boost::get<cell_list>(c));
        break;
      case cell_type::ident:
      {
        auto const ident_it(env.cells.find(boost::get<cell_ident>(c).data));
        if(ident_it != env.cells.end())
        { c = ident_it->second; }
      } break;
      default:
        break;
    }
  }

  auto const env_it(env.funcs.find(func_name));
  if(env_it == env.funcs.end())
  { throw std::runtime_error{ "unknown function: " + func_name }; }

  auto const &func(env_it->second[0].data);
  return func(root);
}
