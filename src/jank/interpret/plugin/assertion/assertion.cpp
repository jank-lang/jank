#include <jank/translate/environment/scope.hpp>
#include <jank/interpret/cell/stream.hpp>
#include <jank/interpret/plugin/assertion/assertion.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/error/internal/exception.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace assertion
      {
        template <typename... Args>
        translate::cell::detail::native_function_declaration
        <translate::cell::cell> find_declaration
        (
          std::shared_ptr<translate::environment::scope> const &trans_scope,
          std::string const &name,
          Args &&...args
        )
        {
          auto const &defs(trans_scope->find_native_function(name));
          if(!defs)
          {
            throw expect::error::internal::exception<>
            { "no native " + name + " declarations" };
          }

          for(auto const &def_pair : defs.value())
          {
            auto const &def(def_pair.first.data);
            if(def.arguments.size() != sizeof...(Args))
            { continue; }

            translate::cell::type_reference const arg_types[]
            { std::forward<Args>(args)... };

            auto const equal
            (
              std::equal
              (
                def.arguments.begin(), def.arguments.end(),
                arg_types.begin(), arg_types.end()
              )
            );

            if(equal)
            { return def; }
          }

          throw expect::error::internal::exception<>
          { "no matching native " + name + " declaration" };
        }

        void assertion
        (
          std::shared_ptr<translate::environment::scope> const &,//trans_scope,
          std::shared_ptr<environment::scope> const &//int_scope
        )
        {
            //[](auto const &scope, auto const &args)
            //{
            //  auto const val
            //  (
            //    interpret::expect::type<interpret::cell::type::boolean>
            //    (interpret::environment::resolve_value(scope, args[0].cell)).data
            //  );
            //  if(!val)
            //  { throw expect::error::assertion::exception<>{}; }
            //  return environment::builtin::value::null();
            //},
        }
      }
    }
  }
}
