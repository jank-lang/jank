#pragma once

#include <stdexcept>
#include <cstdint>
#include <experimental/optional>

#include <jank/translate/cell/trait.hpp>
#include <jank/translate/expect/error/type/exception.hpp>

namespace jank
{
  namespace translate
  {
    namespace expect
    {
      template <cell::type C, typename Cell>
      bool is(Cell const &c)
      { return static_cast<cell::type>(c.which()) == C; }

      template <cell::type C, typename Cell>
      decltype(auto) type(Cell &&c)
      {
        auto const type(static_cast<cell::type>(c.which()));
        if(type != C)
        {
          throw error::type::exception<>
          {
            std::string{ "expected: " } +
            cell::trait::to_string<C>() +
            ", found: " +
            cell::trait::to_string(type)
          };
        }
        return boost::get<cell::trait::to_type<C>>(c);
      }

      /* Returns by copy. */
      template <cell::type C, typename Cell>
      std::experimental::optional<cell::trait::to_type<C>>
      optional_cast(Cell const &c)
      {
        if(is<C>(c))
        { return { type<C>(c) }; }
        else
        { return {}; }
      }

      /* Returns by pointer. */
      template <cell::type C, typename Cell>
      cell::trait::to_type<C>* optional_pointer_cast(Cell &&c)
      {
        if(is<C>(c))
        { return &type<C>(std::forward<Cell>(c)); }
        else
        { return {}; }
      }
    }
  }
}
