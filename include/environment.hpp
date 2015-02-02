#pragma once

#include <map>
#include <string>

#include "cell.hpp"

struct environment
{
  std::map<std::string, cell> cells;
};

environment env
{
  {
    {
      "root",
      cell_func{ [](cell_list const &) -> cell{ return {}; } }
    },
    {
      "+",
      cell_func
      {
        [](cell_list const &cl) -> cell
        {
          auto const list(cl.data);
          if(list.size() < 3)
          { throw std::invalid_argument{ "invalid argument count" }; }

          int val{};
          for(auto i(std::next(list.begin())); i != list.end(); ++i)
          { val += std::stoi(boost::get<cell_string>(*i).data); }

          return cell_string{ std::to_string(val) };
        }
      }
    },
    {
      "print",
      cell_func
      {
        [](cell_list const &cl) -> cell
        {
          auto const list(cl.data);
          if(list.size() < 2)
          { throw std::invalid_argument{ "invalid argument count" }; }

          for(auto i(std::next(list.begin())); i != list.end(); ++i)
          { std::cout << boost::get<cell_string>(*i).data; }
          std::cout << std::endl;

          return {};
        }
      }
    }
  }
};
