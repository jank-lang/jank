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
          inline void make_operator
          (
            std::shared_ptr<environment::scope> const &scope,
            std::string const &name,
            cell::detail::type_reference<cell::cell> const &type
          )
          {
            plugin::detail::make_function
            (
              scope, name,
              environment::builtin::type::boolean(*scope),
              type, type
            );
          }
        }
      }
    }
  }
}
