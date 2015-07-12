#pragma once

#include <jank/translate/plugin/detail/make_function.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace compare
      {
        namespace detail
        {
          template <typename F>
          void make_operator
          (
            std::shared_ptr<environment::scope> const &scope,
            std::string const &name,
            cell::detail::type_reference const &type,
            F const &apply
          )
          {
            plugin::detail::make_function
            (
              scope, name, apply,
              environment::builtin::type::boolean(*scope),
              type, type
            );
          }
        }
      }
    }
  }
}
