#include <jank/translate/function/match_overload.hpp>

#include <jank/parse/expect/type.hpp>
#include <jank/translate/function/argument/resolve_type.hpp>
#include <jank/translate/expect/error/type/overload.hpp>

//namespace jank
//{
//  namespace translate
//  {
//    namespace function
//    {
//      std::experimental::optional<cell::function_call> match_overload
//      (
//        parse::cell::list const &list,
//        std::shared_ptr<environment::scope> const &scope,
//        std::vector<environment::scope::result<cell::function_definition>> const &functions
//      )
//      {
//        auto const arguments(function::argument::call::parse<cell::cell>(list, scope));
//
//        for(auto const &overload_cell : functions)
//        {
//          auto const &overload(overload_cell.first.data);
//
//          if(overload.arguments.size() != arguments.size())
//          { continue; }
//
//          if
//          (
//            std::equal
//            (
//              overload.arguments.begin(), overload.arguments.end(),
//              arguments.begin(),
//              [&](auto const &lhs, auto const &rhs)
//              {
//                return
//                (
//                  lhs.type.definition ==
//                  function::argument::resolve_type(rhs.cell, scope).data
//                );
//              }
//            )
//          )
//          { return { cell::function_call{ { overload_cell.first.data, arguments, scope } } }; }
//        }
//
//        /* No matching overload found. */
//        throw expect::error::type::overload
//        {
//          "no matching function: " +
//          parse::expect::type<parse::cell::type::ident>(list.data[0]).data
//        };
//      }
//    }
//  }
//}
