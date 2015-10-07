#pragma once

#include <vector>

#include <boost/variant.hpp>

namespace jank
{
  namespace translate
  {
    namespace type
    {
      namespace generic
      {
        enum class parameter_type
        {
          single,
          tuple
        };

        template <typename Def>
        struct single
        {
          using type = Def;
          type data;
        };

        template <typename Def>
        bool operator <(single<Def> const &lhs, single<Def> const &rhs)
        { return lhs.data < rhs.data; }

        template <typename Def>
        bool operator ==(single<Def> const &lhs, single<Def> const &rhs)
        { return lhs.data == rhs.data; }

        template <typename Def>
        struct tuple
        {
          using type = std::vector<Def>;
          type data;
        };

        template <typename Def>
        bool operator <(tuple<Def> const &lhs, tuple<Def> const &rhs)
        { return lhs.data < rhs.data; }

        template <typename Def>
        bool operator ==(tuple<Def> const &lhs, tuple<Def> const &rhs)
        { return lhs.data == rhs.data; }

        /* Cells can be types or values. */
        template <typename Def> /* Def = detail::type_definition */
        using parameter = boost::variant
        <
          single<Def>,
          tuple<Def>
        >;
      }
    }
  }
}
