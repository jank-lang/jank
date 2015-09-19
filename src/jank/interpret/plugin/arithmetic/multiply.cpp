#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/interpret/plugin/detail/make_operator.hpp>
#include <jank/interpret/plugin/arithmetic/multiply.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace arithmetic
      {
        void multiply
        (
          std::shared_ptr<translate::environment::scope> const &trans_scope,
          std::shared_ptr<environment::scope> const &int_scope
        )
        {
          detail::make_operator
          (
            trans_scope,
            int_scope,
            "*",
            translate::environment::builtin::type::integer(*trans_scope),
            translate::environment::builtin::type::integer(*trans_scope),
            detail::binary_operator_wrapper
            <
              cell::type::integer,
              cell::type::integer,
              cell::type::integer
            >{},
            [](auto const &l, auto const &r)
            { return l * r; }
          );
          detail::make_operator
          (
            trans_scope,
            int_scope,
            "*",
            translate::environment::builtin::type::real(*trans_scope),
            translate::environment::builtin::type::real(*trans_scope),
            detail::binary_operator_wrapper
            <
              cell::type::real,
              cell::type::real,
              cell::type::real
            >{},
            [](auto const &l, auto const &r)
            { return l * r; }
          );
          detail::make_operator
          (
            trans_scope,
            int_scope,
            "*",
            translate::environment::builtin::type::string(*trans_scope),
            translate::environment::builtin::type::integer(*trans_scope),
            detail::binary_operator_wrapper
            <
              cell::type::string,
              cell::type::integer,
              cell::type::string
            >{},
            [](auto const &l, auto const &r)
            {
              if(r < 0)
              {
                throw expect::error::type::exception<>
                { "invalid string product" };
              }

              interpret::cell::string::type ret{};
              for(auto i(0); i < r; ++i)
              { ret += l; }
              return ret;
            }
          );
        }
      }
    }
  }
}
