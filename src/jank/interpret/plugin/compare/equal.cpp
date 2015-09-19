#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/interpret/plugin/detail/make_operator.hpp>
#include <jank/interpret/plugin/compare/equal.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace compare
      {
        void equal
        (
          std::shared_ptr<translate::environment::scope> const &trans_scope,
          std::shared_ptr<environment::scope> const &int_scope
        )
        {
          detail::make_operator
          (
            trans_scope,
            int_scope,
            "=",
            translate::environment::builtin::type::null(*trans_scope),
            translate::environment::builtin::type::null(*trans_scope),
            detail::binary_operator_wrapper
            <
              cell::type::null,
              cell::type::null,
              cell::type::boolean
            >{},
            [](auto const &, auto const &)
            { return true; }
          );
          detail::make_operator
          (
            trans_scope,
            int_scope,
            "=",
            translate::environment::builtin::type::boolean(*trans_scope),
            translate::environment::builtin::type::boolean(*trans_scope),
            detail::binary_operator_wrapper
            <
              cell::type::boolean,
              cell::type::boolean,
              cell::type::boolean
            >{},
            [](auto const &l, auto const &r)
            { return l == r; }
          );
          detail::make_operator
          (
            trans_scope,
            int_scope,
            "=",
            translate::environment::builtin::type::integer(*trans_scope),
            translate::environment::builtin::type::integer(*trans_scope),
            detail::binary_operator_wrapper
            <
              cell::type::integer,
              cell::type::integer,
              cell::type::boolean
            >{},
            [](auto const &l, auto const &r)
            { return l == r; }
          );
          detail::make_operator
          (
            trans_scope,
            int_scope,
            "=",
            translate::environment::builtin::type::real(*trans_scope),
            translate::environment::builtin::type::real(*trans_scope),
            detail::binary_operator_wrapper
            <
              cell::type::real,
              cell::type::real,
              cell::type::boolean
            >{},
            [](auto const &l, auto const &r)
            { return l == r; }
          );
          detail::make_operator
          (
            trans_scope,
            int_scope,
            "=",
            translate::environment::builtin::type::string(*trans_scope),
            translate::environment::builtin::type::string(*trans_scope),
            detail::binary_operator_wrapper
            <
              cell::type::string,
              cell::type::string,
              cell::type::boolean
            >{},
            [](auto const &l, auto const &r)
            { return l == r; }
          );
        }
      }
    }
  }
}
