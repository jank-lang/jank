#pragma once

#include <map>
#include <string>
#include <memory>
#include <experimental/optional>

#include <jank/parse/cell/cell.hpp>
#include <jank/interpret/function/argument.hpp>
#include <jank/interpret/expect/error/type/type.hpp>

namespace jank
{
  namespace interpret
  {
    namespace environment
    {
      struct scope
      {
        std::experimental::optional<parse::cell::cell> find_cell
        (std::string const &name);
        std::experimental::optional<parse::cell::function> find_function
        (std::string const &name);
        std::experimental::optional<parse::cell::function> find_special
        (std::string const &name);

        std::map<std::string, parse::cell::cell> cells;

        std::map<std::string, std::vector<parse::cell::function>> functions;

        std::map<std::string, parse::cell::function> special_functions;

        // TODO std::shared_ptr<scope> parent;
        scope *parent;
      };
    }
  }

  /* TODO: Move back into parse. */
  namespace parse
  {
    namespace cell
    {
      template <>
      struct wrapper<type::function>
      {
        using type = std::function<cell (interpret::environment::scope&, list const&)>;

        std::vector<interpret::function::argument> arguments;
        type data;
        parse::cell::list body;
        /* TODO: std::shared_ptr<scope> env; */
        interpret::environment::scope env;
      };
    }
  }

  namespace interpret
  {
    namespace environment
    {
      inline std::experimental::optional<jank::parse::cell::cell> scope::find_cell
      (std::string const &name)
      {
        auto const it(cells.find(name));
        if(it == cells.end())
        {
          if(parent)
          { return parent->find_cell(name); }
          else
          { return {}; }
        }
        return { it->second };
      }
      inline std::experimental::optional<jank::parse::cell::function> scope::find_function
      (std::string const &name)
      {
        auto const it(functions.find(name));
        if(it == functions.end())
        {
          if(parent)
          { return parent->find_function(name); }
          else
          { return {}; }
        }

        if(it->second.empty())
        { throw expect::error::type::exception<>{ "unknown function: " + name }; }

        return { it->second[0] };
      }
      inline std::experimental::optional<jank::parse::cell::function> scope::find_special
      (std::string const &name)
      {
        auto const it(special_functions.find(name));
        if(it == special_functions.end())
        {
          if(parent)
          { return parent->find_special(name); }
          else
          { return {}; }
        }
        return { it->second };
      }
    }
  }
}
