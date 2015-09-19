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
          inline void make_operator
          (
            std::shared_ptr<environment::scope> const &scope,
            std::string const &name,
            cell::detail::type_reference<cell::cell> const &type
          )
          {
            plugin::detail::make_function
            (scope, name, type, type, type);
          }

          inline void make_operator
          (
            std::shared_ptr<environment::scope> const &scope,
            std::string const &name,
            cell::detail::type_reference<cell::cell> const &type1,
            cell::detail::type_reference<cell::cell> const &type2,
            cell::detail::type_reference<cell::cell> const &ret_type
          )
          {
            plugin::detail::make_function
            (scope, name, ret_type, type1, type2);
          }
        }
      }
    }
  }
}
