#pragma once

#include <string>
#include <vector>

#include "cell.hpp"
#include "environment/detail/expect.hpp"

struct argument
{
  std::string name;
  cell_type type;
};

inline std::vector<argument> parse_arguments(cell_list const &list)
{
  std::vector<argument> ret;

  for(auto it(list.data.begin()); it != list.data.end(); ++it)
  {
    auto const &name(detail::expect_type<cell_type::ident>(*it).data);
    if(++it == list.data.end())
    { throw std::runtime_error{ "syntax error: expected type after " + name }; }

    auto const &type(detail::expect_type<cell_type::ident>(*it).data);
    ret.push_back({ name, cell_type_from_string(type) });
  }

  return ret;
}
