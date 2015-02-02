#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <boost/variant.hpp>

enum class cell_type
{
  string,
  list,
  function
};

template <cell_type C>
struct cell_wrapper;

using cell = boost::variant
<
  boost::recursive_wrapper<cell_wrapper<cell_type::string>>,
  boost::recursive_wrapper<cell_wrapper<cell_type::list>>,
  boost::recursive_wrapper<cell_wrapper<cell_type::function>>
>;

template <>
struct cell_wrapper<cell_type::string>
{ std::string data; };
template <>
struct cell_wrapper<cell_type::list>
{ std::vector<cell> data; };
template <>
struct cell_wrapper<cell_type::function>
{ std::function<cell (cell_wrapper<cell_type::list> const&)> data; };

using cell_string = cell_wrapper<cell_type::string>;
using cell_list = cell_wrapper<cell_type::list>;
using cell_func = cell_wrapper<cell_type::function>;

int indent_level{ -1 };

std::ostream& operator <<(std::ostream &os, cell const &c)
{
  switch(static_cast<cell_type>(c.which()))
  {
    case cell_type::string:
      os << boost::get<cell_string>(c).data;
      break;
    case cell_type::list:
      ++indent_level;
      os << "\n";
      for(int i{}; i < indent_level; ++i)
      { (void)i; os << "  "; }

      os << "( ";
      for(auto const &v : boost::get<cell_list>(c).data)
      { os << v << " "; }
      os << ") ";

      --indent_level;
      break;
    case cell_type::function:
      os << "function ";
      break;
    default:
      os << "??? ";
  }

  return os;
}
