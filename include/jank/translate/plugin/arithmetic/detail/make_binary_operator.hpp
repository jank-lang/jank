#pragma once

#include <jank/translate/plugin/detail/make_function.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace arithmetic
      {
        namespace detail
        {
          template <typename F>
          void make_binary_operator
          (
            std::shared_ptr<environment::scope> const &scope,
            std::string const &name,
            cell::detail::type_reference const &type,
            F const &apply
          )
          {
            plugin::detail::make_function
            (scope, name, apply, type, type, type);
          }

          template <typename F>
          void make_binary_operator
          (
            std::shared_ptr<environment::scope> const &scope,
            std::string const &name,
            cell::detail::type_reference const &type1,
            cell::detail::type_reference const &type2,
            cell::detail::type_reference const &ret_type,
            F const &apply
          )
          {
            plugin::detail::make_function
            (scope, name, apply, ret_type, type1, type2);
          }
        }
      }
    }
  }
}
