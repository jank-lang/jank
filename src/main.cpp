#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <algorithm>

#include <jtl/iterator/stream_delim.hpp>
#include <jtl/iterator/back_insert.hpp>

#include <jank/interpret/environment/state.hpp>
#include <jank/parse/parse.hpp>
#include <jank/interpret/interpret.hpp>
#include <jank/interpret/environment/prelude/arithmetic.hpp>
#include <jank/interpret/expect/type.hpp>

jank::interpret::environment::state env
{
  { /* cells */
    {
      "forty_two",
      jank::parse::cell::integer{ 42 }
    }
  },
  { /* funcs */
    {
      "root",
      {
        jank::parse::cell::func
        {
          {},
          [](auto const&, jank::parse::cell::list const &)
            -> jank::parse::cell::cell{ return {}; },
          {{}},
          jank::interpret::environment::state()
        }
      }
    },
    {
      "+",
      { jank::parse::cell::func{ {}, &jank::interpret::environment::prelude::sum, {{}}, jank::interpret::environment::state() } }
    },
    {
      "-",
      { jank::parse::cell::func{ {}, &jank::interpret::environment::prelude::difference, {{}}, jank::interpret::environment::state() } }
    },
    {
      "/",
      { jank::parse::cell::func{ {}, &jank::interpret::environment::prelude::quotient, {{}}, jank::interpret::environment::state() } }
    },
    {
      "*",
      { jank::parse::cell::func{ {}, &jank::interpret::environment::prelude::product, {{}}, jank::interpret::environment::state() } }
    },
    {
      "print",
      {
        jank::parse::cell::func
        {
          {},
          [](auto const&, jank::parse::cell::list const &cl)
            -> jank::parse::cell::cell
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
          {{}},
          jank::interpret::environment::state()
        }
      }
    }
  },
  { /* special */
    {
      "func",
      jank::parse::cell::func
      {
        {},
        [](auto &env, jank::parse::cell::list const &cl)
          -> jank::parse::cell::cell
        {
          auto const &list(cl.data);
          auto const name(jank::interpret::expect::type<jank::parse::cell::type::ident>(list[1]));
          auto const args(jank::interpret::expect::type<jank::parse::cell::type::list>(list[2]));
          auto const ret(jank::interpret::expect::type<jank::parse::cell::type::list>(list[3]));

          auto &overloads(env.funcs[name.data]);
          auto arguments(jank::interpret::function::parse_arguments(args));

          /* Prevent redefinition. */
          if(overloads.size())
          {
            if(std::find_if(overloads.begin(), overloads.end(),
                            [&](auto const &e)
                            { return e.arguments == arguments; })
               != overloads.end())
            {
              throw jank::interpret::expect::error::type::type<>
              { "function already defined: " + name.data };
            }
            std::cout << "overloading function: " << name.data << std::endl;
          }

          overloads.emplace_back();
          jank::parse::cell::func &func{ overloads.back() };
          func.arguments = std::move(arguments);
          func.env.parent = &env;
          std::copy(std::next(list.begin(), 4), list.end(),
                    std::back_inserter(func.body.data));

          func.data = [=, &overloads](auto &, jank::parse::cell::list const &args)
            -> jank::parse::cell::cell
          {
            for(auto const &func : overloads)
            {
              std::cout << "function: " << name.data << "\n"
                        << "\targuments: ";
              for(auto const &arg : func.arguments)
              {
                std::cout << arg.name << " : "
                          << jank::parse::cell::type_string(arg.type)
                          << "; ";
              }
              std::cout << std::endl;
            }

            for(auto func : overloads)
            {
              if(args.data.size() - 1 != func.arguments.size())
              { continue; }

              std::size_t matched{};
              for(auto const i : jtl::it::make_range(func.arguments.size()))
              {
                auto const expected_type(func.arguments[i].type);
                auto const found_type
                (static_cast<jank::parse::cell::type>(args.data[i + 1].which()));

                std::cout << "expected: "
                          << jank::parse::cell::type_string(expected_type)
                          << ", found: "
                          << jank::parse::cell::type_string(found_type)
                          << std::endl;
                if(expected_type == found_type)
                {
                  func.env.cells[func.arguments[i].name] = args.data[i + 1];
                  ++matched;
                }
                else
                { continue; }
              }
              if(matched != func.arguments.size())
              { continue; }

              std::cout << "selected: ";
              for(auto const &arg : func.arguments)
              {
                std::cout << arg.name << " : "
                          << jank::parse::cell::type_string(arg.type)
                          << "; ";
              }
              std::cout << std::endl;

              jank::parse::cell::list body{ { jank::parse::cell::ident{ "root" } } };
              std::copy(func.body.data.begin(), func.body.data.end(),
                        std::back_inserter(body.data));
              return jank::interpret::interpret(func.env, body);
            }
            throw jank::interpret::expect::error::type::overload
            { "invalid function call to function: " + name.data };
          };

          return {};
        },
        {{}},
        jank::interpret::environment::state()
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

  /* TODO: translate */

  /* TODO: Keep all interpret shit in cells or AST cells? */
  jank::interpret::interpret
  (
    env,
    jank::interpret::expect::type<jank::parse::cell::type::list>(root)
  );
}
