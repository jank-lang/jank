#pragma once

#include <stdexcept>
#include <cstdint>
#include <experimental/optional>

#include <jank/parse/cell/cell.hpp>
#include <jank/parse/cell/trait.hpp>
#include <jank/interpret/expect/error/type/type.hpp>

namespace jank
{
  namespace interpret
  {
    namespace expect
    {
      template <parse::cell::type C, typename Cell>
      bool is(Cell const &c)
      { return static_cast<parse::cell::type>(c.which()) == C; }

      template <parse::cell::type C, typename Cell>
      decltype(auto) type(Cell &&c)
      {
        auto const type(static_cast<parse::cell::type>(c.which()));
        if(type != C)
        {
          throw error::type::exception<>
          {
            std::string{ "expected: " } +
            parse::cell::trait::to_string<C>() +
            ", found: " +
            parse::cell::trait::to_string(type)
          };
        }
        return boost::get<parse::cell::trait::to_type<C>>(c);
      }

      template <parse::cell::type C, typename Cell>
      std::experimental::optional<parse::cell::trait::to_type<C>>
      optional_cast(Cell const &c)
      {
        if(is<C>(c))
        { return { type<C>(c) }; }
        else
        { return {}; }
      }
    }
  }
}
