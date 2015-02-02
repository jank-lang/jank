#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <cstdint>

#include <boost/variant.hpp>

/* TODO: ident, real, etc. */
enum class cell_type
{
  integer,
  string,
  list,
  function /* TODO: implement. */
};

template <cell_type C>
struct cell_wrapper;

using cell = boost::variant
<
  boost::recursive_wrapper<cell_wrapper<cell_type::integer>>,
  boost::recursive_wrapper<cell_wrapper<cell_type::string>>,
  boost::recursive_wrapper<cell_wrapper<cell_type::list>>,
  boost::recursive_wrapper<cell_wrapper<cell_type::function>>
>;

template <>
struct cell_wrapper<cell_type::integer>
{
  using type = int64_t;
  type data;
};
template <>
struct cell_wrapper<cell_type::string>
{
  using type = std::string;
  type data;
};
template <>
struct cell_wrapper<cell_type::list>
{
  using type = std::vector<cell>;
  type data;
};
template <>
struct cell_wrapper<cell_type::function>
{
  using type = std::function<cell (cell_wrapper<cell_type::list> const&)>;
  type data;
};

using cell_int = cell_wrapper<cell_type::integer>;
using cell_string = cell_wrapper<cell_type::string>;
using cell_list = cell_wrapper<cell_type::list>;
using cell_func = cell_wrapper<cell_type::function>;

std::ostream& operator <<(std::ostream &os, cell const &c)
{
  static int indent_level{ -1 };

  switch(static_cast<cell_type>(c.which()))
  {
    case cell_type::integer:
      os << boost::get<cell_int>(c).data;
      break;
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
