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
        auto apply(cell::cell_int::type const to, cell::cell_int::type const from);
        template <>
        inline auto apply<op::add>(cell::cell_int::type const to, cell::cell_int::type const from)
        { return to + from; }
        template <>
        inline auto apply<op::sub>(cell::cell_int::type const to, cell::cell_int::type const from)
        { return to - from; }
        template <>
        inline auto apply<op::mul>(cell::cell_int::type const to, cell::cell_int::type const from)
        { return to * from; }
        template <>
        inline auto apply<op::div>(cell::cell_int::type const to, cell::cell_int::type const from)
        { return to / from; }

        /* TODO: read idents from env */
        template <op O>
        cell::cell apply_all(environment&, cell::cell_list const &cl)
        {
          auto const list(cl.data);
          jank::environment::detail::expect_at_least_args(cl, 2);

          cell::cell_int::type val
          {
            jank::environment::detail::expect_type<cell::cell_type::integer>
            (
              *std::next(list.begin())
            ).data
          };
          for(auto &i : jtl::it::make_range(std::next(list.begin(), 2), list.end()))
          { val = apply<O>(val, jank::environment::detail::expect_type<cell::cell_type::integer>(i).data); }

          return cell::cell_int{ val };
        }
      }

      inline cell::cell sum(environment &env, cell::cell_list const &cl)
      { return detail::apply_all<detail::op::add>(env, cl); }

      inline cell::cell difference(environment &env, cell::cell_list const &cl)
      { return detail::apply_all<detail::op::sub>(env, cl); }

      inline cell::cell product(environment &env, cell::cell_list const &cl)
      { return detail::apply_all<detail::op::mul>(env, cl); }

      inline cell::cell quotient(environment &env, cell::cell_list const &cl)
      { return detail::apply_all<detail::op::div>(env, cl); }
    }
  }
}
