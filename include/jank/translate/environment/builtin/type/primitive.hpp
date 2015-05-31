#pragma once

#include <jank/translate/environment/scope.hpp>
#include <jank/translate/cell/detail/type_reference.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace builtin
      {
        namespace type
        {
          std::shared_ptr<scope> add_primitives(std::shared_ptr<scope> const &s);

          cell::detail::type_reference null(std::shared_ptr<scope> const &s);
          cell::detail::type_reference boolean(std::shared_ptr<scope> const &s);
          cell::detail::type_reference integer(std::shared_ptr<scope> const &s);
          cell::detail::type_reference real(std::shared_ptr<scope> const &s);
          cell::detail::type_reference string(std::shared_ptr<scope> const &s);
        }
      }
    }
  }
}
