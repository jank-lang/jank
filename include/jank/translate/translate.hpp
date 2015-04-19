#pragma once

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/special/all.hpp>

namespace jank
{
  namespace translate
  {
    template <typename Range>
    cell::function_body translate(Range const &range, std::shared_ptr<environment::scope> const &scope)
    {
      if(!std::distance(range.begin(), range.end()))
      { return { { {}, {} } }; }

      cell::function_body translated{ { {}, scope } };
      std::for_each
      (
        range.begin(), range.end(),
        [&](auto const &c)
        {
          /* Handle specials. */
          if(expect::is<parse::cell::type::list>(c))
          {
            auto const opt
            (
              environment::special::handle
              (expect::type<parse::cell::type::list>(c), translated)
            );
            if(opt)
            { translated.data.cells.push_back(opt.value()); }
          }

          /* TODO: Handle plain values (only useful at the end of a function? */
          /* TODO: Handle function calls. */
        }
      );
      return translated;
      //auto const &func_name(expect::type<parse::cell::type::ident>(root.data[0]).data);

      //auto const special_it(env.find_special(func_name));
      //if(special_it)
      //{ return special_it->data(env, root); }

      ///* Collapse all nested lists to values. */
      //for(auto &c : root.data)
      //{
      //  switch(static_cast<parse::cell::type>(c.which()))
      //  {
      //    case parse::cell::type::list:
      //      c = interpret(env, boost::get<parse::cell::list>(c));
      //      break;
      //    case parse::cell::type::ident:
      //    {
      //      auto const ident_it(env.find_cell(boost::get<parse::cell::ident>(c).data));
      //      if(ident_it)
      //      { c = *ident_it; }
      //    } break;
      //    default:
      //      break;
      //  }
      //}

      //auto const env_it(env.find_function(func_name));
      //if(!env_it)
      //{ throw expect::error::type::type<>{ "unknown function: " + func_name }; }

      //auto const &func(env_it->data);
      //return func(env, root);
    }
  }
}
