#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <algorithm>

#include <jtl/iterator/stream_delim.hpp>
#include <jtl/iterator/back_insert.hpp>

#include <jank/environment/state.hpp>
#include <jank/parse/parse.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/environment/prelude/arithmetic.hpp>
#include <jank/expect/type.hpp>

jank::environment::state env
{
  { /* cells */
    {
      "forty_two",
      jank::cell::integer{ 42 }
    }
  },
  { /* funcs */
    {
      "root",
      {
        jank::cell::func
        {
          {},
          [](auto const&, jank::cell::list const &) -> jank::cell::cell{ return {}; },
          jank::environment::state()
        }
      }
    },
    {
      "+",
      { jank::cell::func{ {}, &jank::environment::prelude::sum, jank::environment::state() } }
    },
    {
      "-",
      { jank::cell::func{ {}, &jank::environment::prelude::difference, jank::environment::state() } }
    },
    {
      "/",
      { jank::cell::func{ {}, &jank::environment::prelude::quotient, jank::environment::state() } }
    },
    {
      "*",
      { jank::cell::func{ {}, &jank::environment::prelude::product, jank::environment::state() } }
    },
    {
      "print",
      {
        jank::cell::func
        {
          {},
          [](auto const&, jank::cell::list const &cl) -> jank::cell::cell
          {
            auto const &list(cl.data);

            /* TODO: remove; use a proper signature */
            if(list.size() < 2)
            { throw std::invalid_argument{ "invalid argument count" }; }

            for(auto i(std::next(list.begin())); i != list.end(); ++i)
            { std::cout << *i; }
            std::cout << std::endl;

            return {};
          },
          jank::environment::state()
        }
      }
    }
  },
  { /* special */
    {
      "func",
      jank::cell::func
      {
        {},
        [](auto &env, jank::cell::list const &cl) -> jank::cell::cell
        {
          auto const &list(cl.data);
          auto const name(jank::expect::type<jank::cell::type::ident>(list[1]));
          auto const args(jank::expect::type<jank::cell::type::list>(list[2]));
          auto const ret(jank::expect::type<jank::cell::type::list>(list[3]));

          if(env.funcs[name.data].size())
          {
            throw jank::expect::error::type::type<>
            { "function already defined " + name.data };
          }
          env.funcs[name.data].emplace_back();

          jank::cell::func &func{ env.funcs[name.data].back() };
          func.arguments = jank::function::parse_arguments(args);
          func.env.parent = &env;
          func.data = [=, &func](auto &, jank::cell::list const &args)
            -> jank::cell::cell
          {
            if(args.data.size() - 1 != func.arguments.size())
            {
              throw jank::expect::error::type::overload
              {
                "invalid argument count (expected " +
                std::to_string(func.arguments.size()) +
                ", found " + std::to_string(args.data.size() - 1) +
                ")"
              };
            }
            for(auto const i : jtl::it::make_range(func.arguments.size()))
            {
              auto const expected_type(func.arguments[i].type);
              auto const found_type
              (static_cast<jank::cell::type>(args.data[i + 1].which()));

              if(expected_type == found_type)
              { func.env.cells[func.arguments[i].name] = args.data[i + 1]; }
              else
              {
                throw jank::expect::error::type::overload
                {
                  std::string{ "invalid argument type (expected " } +
                  jank::cell::type_string(expected_type) +
                  ", found " + jank::cell::type_string(found_type) +
                  ")"
                };
              }
            }

            jank::cell::list body{ { jank::cell::ident{ "root" } } };
            std::copy(std::next(list.begin(), 4), list.end(),
                      std::back_inserter(body.data));
            return jank::interpret::interpret(func.env, body);
          };

          return {};
        },
        jank::environment::state()
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
    jank::parse::parse
    (
      {
        std::istreambuf_iterator<char>{ ifs },
        std::istreambuf_iterator<char>{}
      }
    )
  );

  std::cout << root << std::endl;

  jank::interpret::interpret
  (
    env,
    jank::expect::type<jank::cell::type::list>(root)
  );
}
