#pragma once

#include <stdexcept>
#include <experimental/optional>

#include <jank/interpret/cell/cell.hpp>
#include <jank/interpret/cell/trait.hpp>
#include <jank/interpret/expect/error/type/exception.hpp>

namespace jank
{
  namespace interpret
  {
    namespace expect
    {
      template <cell::type C, typename Cell>
      bool is(Cell const &c)
      { return cell::trait::to_enum(c) == C; }

      template <cell::type C, typename Cell>
      decltype(auto) type(Cell &&c)
      {
        auto const type(cell::trait::to_enum(c));
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

      template <cell::type C, typename Cell>
      std::experimental::optional<cell::trait::to_type<C>>
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
