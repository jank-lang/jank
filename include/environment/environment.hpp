#pragma once

#include <map>
#include <string>

#include "prelude/arithmetic.hpp"

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
      cell_func{ &sum }
    },
    {
      "-",
      cell_func{ &difference }
    },
    {
      "/",
      cell_func{ &quotient }
    },
    {
      "*",
      cell_func{ &product }
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
          { std::cout << *i; }
          std::cout << std::endl;

          return {};
        }
      }
    }
  }
};
