#pragma once

#include <map>
#include <string>

#include "prelude/arithmetic.hpp"

cell interpret(cell_list &root);

struct environment
{
  std::map<std::string, cell> cells;
  
  /* TODO: map<string, vector<cell_func>> for overloading.
   * Each cell_func has a vector<cell_type> for the args.
   * Calling a function first type checks each overload. */
  std::map<std::string, std::vector<cell_func>> funcs;

  std::map<std::string, cell_func> special_funcs;
};

environment env /* TODO: fix this global shit */
{
  { /* cells */
  },
  { /* funcs */
    {
      "root",
      {
        cell_func
        {
          {},
          [](cell_list const &) -> cell{ return {}; }
        }
      }
    },
    {
      "+",
      { cell_func{ {}, &sum } }
    },
    {
      "-",
      { cell_func{ {}, &difference } }
    },
    {
      "/",
      { cell_func{ {}, &quotient } }
    },
    {
      "*",
      { cell_func{ {}, &product } }
    },
    {
      "print",
      {
        cell_func
        {
          {},
          [](cell_list const &cl) -> cell
          {
            auto const &list(cl.data);
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
  },
  { /* special */
    {
      "func",
      cell_func
      {
        {},
        [](cell_list const &cl) -> cell
        {
          std::cout << "func declaration:" << std::endl;
          auto const &list(cl.data);

          auto const name(detail::expect_type<cell_type::ident>(list[1]));
          std::cout << " name: " << name << std::endl;

          auto const args(detail::expect_type<cell_type::list>(list[2]));
          std::cout << " args: " << args << std::endl;

          auto const ret(detail::expect_type<cell_type::list>(list[3]));
          std::cout << " ret: " << ret << std::endl;

          for
          (
            auto const &l :
            jtl::it::make_range
            (
              std::next(list.begin(), 4),
              list.end()
            )
          )
          { std::cout << "  body: " << l << std::endl; }

          env.funcs[name.data] =
          {
            cell_func
            {
              {},
              [=](cell_list const &) -> cell
              {
                cell_list body{ { cell_ident{ "root" } } };
                std::copy(std::next(list.begin(), 4), list.end(),
                          std::back_inserter(body.data));
                return interpret(body);
              }
            }
          };

          return {};
        }
      }
    }
  }
};
