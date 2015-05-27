#pragma once

#include <jank/interpret/environment/scope.hpp>
#include <jank/interpret/environment/prelude/arithmetic.hpp>

namespace jank
{
  namespace interpret
  {
    namespace environment
    {
      namespace prelude
      {
        inline auto& env()
        {
          static scope env
          {
            { /* cells */
              {
                "forty_two",
                jank::parse::cell::integer{ 42 }
              }
            },
            { /* functions */
              {
                "root",
                {
                  jank::parse::cell::function
                  {
                    {},
                    [](auto const&, jank::parse::cell::list const &)
                      -> jank::parse::cell::cell{ return {}; },
                    {{}},
                    jank::interpret::environment::scope()
                  }
                }
              },
              {
                "+",
                { jank::parse::cell::function{ {}, &jank::interpret::environment::prelude::sum, {{}}, jank::interpret::environment::scope() } }
              },
              {
                "-",
                { jank::parse::cell::function{ {}, &jank::interpret::environment::prelude::difference, {{}}, jank::interpret::environment::scope() } }
              },
              {
                "/",
                { jank::parse::cell::function{ {}, &jank::interpret::environment::prelude::quotient, {{}}, jank::interpret::environment::scope() } }
              },
              {
                "*",
                { jank::parse::cell::function{ {}, &jank::interpret::environment::prelude::product, {{}}, jank::interpret::environment::scope() } }
              },
              {
                "print",
                {
                  jank::parse::cell::function
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
                    jank::interpret::environment::scope()
                  }
                }
              }
            },
            { /* special */
              {
                "function",
                jank::parse::cell::function
                {
                  {},
                  [](auto &env, jank::parse::cell::list const &cl)
                    -> jank::parse::cell::cell
                  {
                    auto const &list(cl.data);
                    auto const name(jank::interpret::expect::type<jank::parse::cell::type::ident>(list[1]));
                    auto const args(jank::interpret::expect::type<jank::parse::cell::type::list>(list[2]));
                    auto const ret(jank::interpret::expect::type<jank::parse::cell::type::list>(list[3]));

                    auto &overloads(env.functions[name.data]);
                    auto arguments(jank::interpret::function::parse_arguments(args));

                    /* Prevent redefinition. */
                    if(overloads.size())
                    {
                      if(std::find_if(overloads.begin(), overloads.end(),
                                      [&](auto const &e)
                                      { return e.arguments == arguments; })
                         != overloads.end())
                      {
                        throw jank::interpret::expect::error::type::overload
                        { "function already defined: " + name.data };
                      }
                    }

                    overloads.emplace_back();
                    jank::parse::cell::function &func{ overloads.back() };
                    func.arguments = std::move(arguments);
                    func.env.parent = &env;
                    std::copy(std::next(list.begin(), 4), list.end(),
                              std::back_inserter(func.body.data));

                    func.data = [=, &overloads](auto &, jank::parse::cell::list const &args)
                      -> jank::parse::cell::cell
                    {
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
                  jank::interpret::environment::scope()
                }
              }
            },
            nullptr /* no parent */
          };

          return env;
        }
      }
    }
  }
}
