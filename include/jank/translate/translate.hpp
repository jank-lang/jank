#pragma once

#include <jtl/iterator/range.hpp>

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/translate/environment/scope.hpp>

namespace jank
{
  namespace translate
  {
    cell::function_body translate
    (
      jtl::it::range::indirect
      <parse::cell::list::type::const_iterator> const &range,
      std::shared_ptr<environment::scope> const &scope,
      cell::function_body translated
    );

    cell::function_body translate
    (
      jtl::it::range::indirect
      <parse::cell::list::type::const_iterator> const &range,
      std::shared_ptr<environment::scope> const &scope,
      cell::type_reference return_type
    );
  }
}
