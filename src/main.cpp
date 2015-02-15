#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <algorithm>

#include <jtl/iterator/stream_delim.hpp>
#include <jtl/iterator/back_insert.hpp>

#include "environment/environment.hpp"
#include "parse.hpp"
#include "interpret.hpp"
#include "environment/prelude/arithmetic.hpp"

environment env /* TODO: fix this global shit */
{
  { /* cells */
    {
      "forty_two",
      cell_int{ 42 }
    }
  },
  { /* funcs */
    {
      "root",
      {
        cell_func
        {
          {},
          [](auto const&, cell_list const &) -> cell{ return {}; },
          environment()
        }
      }
    },
    {
      "+",
      { cell_func{ {}, &sum, environment() } }
    },
    {
      "-",
      { cell_func{ {}, &difference, environment() } }
    },
    {
      "/",
      { cell_func{ {}, &quotient, environment() } }
    },
    {
      "*",
      { cell_func{ {}, &product, environment() } }
    },
    {
      "print",
      {
        cell_func
        {
          {},
          [](auto const&, cell_list const &cl) -> cell
          {
            auto const &list(cl.data);
            if(list.size() < 2)
            { throw std::invalid_argument{ "invalid argument count" }; }

            for(auto i(std::next(list.begin())); i != list.end(); ++i)
            { std::cout << *i; }
            std::cout << std::endl;

            return {};
          },
          environment()
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
        [](auto &env, cell_list const &cl) -> cell
        {
          auto const &list(cl.data);
          auto const name(detail::expect_type<cell_type::ident>(list[1]));
          auto const args(detail::expect_type<cell_type::list>(list[2]));
          auto const ret(detail::expect_type<cell_type::list>(list[3]));

          if(env.funcs[name.data].size())
          { throw std::runtime_error{ "function already defined: " + name.data }; }
          env.funcs[name.data].emplace_back();

          /* TODO: sort out this env shit */
          cell_func &func{ env.funcs[name.data].back() };
          func.env.parent = &env;
          func.data = [=, &func](auto &, cell_list const &args) -> cell
          {
            /* TODO: merge args into function's env before interpreting */
            for
            (
              auto const &a :
              jtl::it::make_range(std::next(args.data.begin()), args.data.end())
            )
            { std::cout << "arg: " << a << std::endl; }

            cell_list body{ { cell_ident{ "root" } } };
            std::copy(std::next(list.begin(), 4), list.end(),
                      std::back_inserter(body.data));
            return interpret(func.env, body);
          };

          return {};
        },
        environment()
      }
    }
  },
  nullptr /* no parent */
};

int main(int const argc, char ** const argv)
{
  if(argc != 2)
  {
    std::cout << "usage: " << argv[0] << " <file>" << std::endl;
    return 1;
  }

  std::ifstream ifs{ argv[1] };

  auto root
  (
    parse
    (
      {
        std::istreambuf_iterator<char>{ ifs },
        std::istreambuf_iterator<char>{}
      }
    )
  );

  std::cout << root << std::endl;

  interpret(env, boost::get<cell_list>(root));
}
