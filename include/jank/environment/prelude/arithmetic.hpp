#pragma once

#include <jtl/iterator/range.hpp>

#include <jank/cell/cell.hpp>
#include <jank/environment/detail/expect.hpp>

namespace jank
{
  namespace environment
  {
    namespace prelude
    {
      namespace detail
      {
        enum class op
        {
          add,
          sub,
          mul,
          div
        };

        template <op O>
        auto apply(cell::integer::type const to, cell::integer::type const from);
        template <>
        inline auto apply<op::add>(cell::integer::type const to, cell::integer::type const from)
        { return to + from; }
        template <>
        inline auto apply<op::sub>(cell::integer::type const to, cell::integer::type const from)
        { return to - from; }
        template <>
        inline auto apply<op::mul>(cell::integer::type const to, cell::integer::type const from)
        { return to * from; }
        template <>
        inline auto apply<op::div>(cell::integer::type const to, cell::integer::type const from)
        { return to / from; }

        /* TODO: read idents from env */
        template <op O>
        cell::cell apply_all(environment&, cell::list const &cl)
        {
          auto const list(cl.data);
          jank::environment::detail::expect_at_least_args(cl, 2);

          cell::integer::type val
          {
            jank::environment::detail::expect_type<cell::type::integer>
            (
              *std::next(list.begin())
            ).data
          };
          for(auto &i : jtl::it::make_range(std::next(list.begin(), 2), list.end()))
          { val = apply<O>(val, jank::environment::detail::expect_type<cell::type::integer>(i).data); }

          return cell::integer{ val };
        }
      }

      inline cell::cell sum(environment &env, cell::list const &cl)
      { return detail::apply_all<detail::op::add>(env, cl); }

      inline cell::cell difference(environment &env, cell::list const &cl)
      { return detail::apply_all<detail::op::sub>(env, cl); }

      inline cell::cell product(environment &env, cell::list const &cl)
      { return detail::apply_all<detail::op::mul>(env, cl); }

      inline cell::cell quotient(environment &env, cell::list const &cl)
      { return detail::apply_all<detail::op::div>(env, cl); }
    }
  }
}
