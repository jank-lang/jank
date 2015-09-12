#include <sstream>

#include <jank/parse/expect/type.hpp>
#include <jank/translate/function/match_indirect.hpp>
#include <jank/translate/function/argument/resolve_type.hpp>
#include <jank/translate/expect/error/type/overload.hpp>
#include <jank/translate/expect/error/internal/exception.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      void match_indirect
      (
        cell::binding_definition const &binding,
        parse::cell::list const &args,
        std::shared_ptr<environment::scope> const &scope,
        std::function<void (cell::indirect_function_call)> const &callback
      )
      {
        auto const arguments
        (function::argument::call::parse<cell::cell>(args, scope));

        auto const &generics(binding.data.type.definition.generics);
        if(generics.parameters.size() != 2)
        { throw expect::error::internal::exception<>{ "invalid generic" }; }
        auto const &expected_args
        (
          boost::get
          <
            type::generic::tuple<cell::detail::type_definition<cell::cell>>
          >(generics.parameters[0])
        );

        if
        (
          arguments.size() == expected_args.data.size() &&
          std::equal
          (
            arguments.begin(), arguments.end(),
            expected_args.data.begin(),
            [&](auto const &arg, auto const &expected)
            { return argument::resolve_type(arg.cell, scope).data == expected; }
          )
        )
        {
          callback({ { binding.data, arguments } });
          return;
        }

        std::stringstream ss;
        ss << "no matching function: "
           << parse::expect::type<parse::cell::type::ident>(args.data[0]).data
           << " with arguments: ";

        for(auto const &arg : arguments)
        {
          ss << arg.name << " : "
             << function::argument::resolve_type(arg.cell, scope).data.name
             << " ";
        }
        throw expect::error::type::overload
        { ss.str() };
      }
    }
  }
}
