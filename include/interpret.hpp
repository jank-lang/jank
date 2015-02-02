#pragma once

#include "cell.hpp"

cell interpret(cell_list &root)
{
  /* Collapse all nested lists to values. */
  for(auto &c : root.data)
  {
    if(c.which() == static_cast<int>(cell_type::list))
    { c = interpret(boost::get<cell_list>(c)); }
  }

  auto const &func_name(boost::get<cell_string>(root.data[0]).data);
  auto const env_it(env.cells.find(func_name));
  if(env_it == env.cells.end())
  { throw std::runtime_error{ "unknown function: " + func_name }; }

  auto const &func(boost::get<cell_func>(env_it->second).data);
  return func(root);
}
