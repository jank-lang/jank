#pragma once

#include <jank/interpret/cell/trait.hpp>
#include <jank/interpret/plugin/detail/make_function.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/type.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace detail
      {
        /* Used for easy type specification. */
        template <cell::type T1, cell::type T2, cell::type Ret>
        struct binary_operator_wrapper
        { };
        template <cell::type T1, cell::type Ret>
        struct unary_operator_wrapper
        { };

        /* Helper wrapper around make_function for binary ops. */
        template <typename F, cell::type T1, cell::type T2, cell::type Ret>
        void make_operator
        (
          std::shared_ptr<translate::environment::scope> const &trans_scope,
          std::shared_ptr<environment::scope> const &int_scope,
          std::string const &name,
          translate::cell::detail::type_reference
          <translate::cell::cell> const &type1,
          translate::cell::detail::type_reference
          <translate::cell::cell> const &type2,
          binary_operator_wrapper<T1, T2, Ret> const&,
          F const &apply
        )
        {
          make_function
          (
            trans_scope, int_scope,
            name,
            [apply](auto const &scope, auto const &args)
            {
              auto const v1
              (
                expect::type<T1>
                (environment::resolve_value(scope, args[0].cell)).data
              );
              auto const v2
              (
                expect::type<T2>
                (environment::resolve_value(scope, args[1].cell)).data
              );
              return cell::cell{ cell::trait::to_type<Ret>{ apply(v1, v2) } };
            },
            type1, type2
          );
        }

        template <typename F, cell::type T1, cell::type Ret>
        void make_operator
        (
          std::shared_ptr<translate::environment::scope> const &trans_scope,
          std::shared_ptr<environment::scope> const &int_scope,
          std::string const &name,
          translate::cell::detail::type_reference
          <translate::cell::cell> const &type1,
          unary_operator_wrapper<T1, Ret> const&,
          F const &apply
        )
        {
          make_function
          (
            trans_scope, int_scope,
            name,
            [apply](auto const &scope, auto const &args)
            {
              auto const v1
              (
                expect::type<T1>
                (environment::resolve_value(scope, args[0].cell)).data
              );
              return cell::cell{ cell::trait::to_type<Ret>{ apply(v1) } };
            },
            type1
          );
        }
      }
    }
  }
}
