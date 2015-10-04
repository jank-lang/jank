#pragma once

#include <sstream>

#include <jank/parse/expect/type.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/function/argument/resolve_type.hpp>
#include <jank/translate/expect/error/type/overload.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace detail
      {
        template <typename Def>
        struct call
        { using type = cell::function_call; };
        template <>
        struct call<cell::native_function_declaration::type>
        { using type = cell::native_function_call; };
        template <>
        struct call<cell::macro_definition::type>
        { using type = cell::macro_call; };

        template <typename Func>
        auto make_call
        (
          Func const &func,
          function::argument::value_list<cell::cell> arguments,
          std::shared_ptr<environment::scope> const &scope
        )
        {
          return boost::optional<typename detail::call<Func>::type>
          { { { func, arguments, scope } } };
        }
        template <>
        inline auto make_call<cell::macro_definition::type>
        (
          cell::macro_definition::type const &func,
          function::argument::value_list<cell::cell> arguments,
          std::shared_ptr<environment::scope> const&
        )
        {
          return boost::optional<cell::macro_call>
          { { { func, arguments, {} } } };
        }

        /* Handles native and non-native functions. */
        template <typename Def>
        boost::optional<typename detail::call<typename Def::type>::type>
        match_overload
        (
          function::argument::value_list<cell::cell> arguments,
          std::shared_ptr<environment::scope> const &scope,
          std::vector<environment::scope::result<Def>> const &functions
        )
        {
          for(auto const &overload_cell : functions)
          {
            auto const &overload(overload_cell.first.data);

            if(overload.arguments.size() != arguments.size())
            { continue; }

            if
            (
              std::equal
              (
                overload.arguments.begin(), overload.arguments.end(),
                arguments.begin(),
                [&](auto const &lhs, auto const &rhs)
                {
                  return
                  (
                    lhs.type.definition ==
                    function::argument::resolve_type(rhs.cell, scope).data
                  );
                }
              )
            )
            { return make_call(overload, arguments, scope); }
          }
          return {};
        }
      }

      template <typename Macro,
                typename Native, typename Non_Native,
                typename Callback>
      void match_overload
      (
        parse::cell::list const &list,
        std::shared_ptr<environment::scope> const &scope,
        Macro const &macro,
        Native const &native,
        Non_Native const &non_native,
        Callback const &callback
      )
      {
        auto const arguments
        (function::argument::call::parse<cell::cell>(list, scope));

        auto const match
        (
          [&](auto const &opt)
          {
            if(!opt)
            { return false; }

            auto const matched_opt
            (detail::match_overload(arguments, scope, *opt));
            if(matched_opt)
            { callback(*matched_opt); }
            return static_cast<bool>(matched_opt);
          }
        );

        if(!match(macro) && !match(non_native) && !match(native))
        {
          /* No matching overload found. */
          std::stringstream ss;
          ss << "no matching call: "
             << parse::expect::type<parse::cell::type::ident>(list.data[0]).data
             << " with arguments: ";

          for(auto const &arg : arguments)
          {
            ss << "(" << arg.name << " "
               << function::argument::resolve_type(arg.cell, scope).data.name
               << ") ";
          }
          throw expect::error::type::overload
          { ss.str() };
        }
      }
    }
  }
}
