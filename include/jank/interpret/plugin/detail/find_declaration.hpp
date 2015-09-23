#pragma once

#include <jank/translate/cell/detail/type_reference.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/value/primitive.hpp>
#include <jank/interpret/expect/error/internal/exception.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace detail
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

          for(auto const &def_pair : *defs)
          {
            auto const &def(def_pair.first.data);
            if(def.arguments.size() != sizeof...(Args))
            { continue; }

            translate::cell::detail::type_reference
            <translate::cell::cell> const arg_types[]
            {
              std::forward<Args>(args)...,

              /* XXX: Hack to allow zero-size args; this will be ignored. */
              translate::environment::builtin::type::null(*trans_scope)
            };

            auto const equal
            (
              std::equal
              (
                def.arguments.begin(), def.arguments.end(),
                std::begin(arg_types), std::next(std::end(arg_types), -1),
                [](auto const &lhs, auto const &rhs)
                { return lhs.type == rhs; }
              )
            );

            if(equal)
            { return def; }
          }

          throw expect::error::internal::exception<>
          { "no matching native " + name + " declaration" };
        }
      }
    }
  }
}
