#pragma once

#include <jtl/iterator/range.hpp>

#include <jank/parse/cell/cell.hpp>
#include <jank/interpret/expect/type.hpp>
#include <jank/interpret/expect/argument.hpp>

namespace jank
{
  namespace interpret
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
          auto apply(parse::cell::integer::type const to, parse::cell::integer::type const from);
          template <>
          inline auto apply<op::add>(parse::cell::integer::type const to, parse::cell::integer::type const from)
          { return to + from; }
          template <>
          inline auto apply<op::sub>(parse::cell::integer::type const to, parse::cell::integer::type const from)
          { return to - from; }
          template <>
          inline auto apply<op::mul>(parse::cell::integer::type const to, parse::cell::integer::type const from)
          { return to * from; }
          template <>
          inline auto apply<op::div>(parse::cell::integer::type const to, parse::cell::integer::type const from)
          { return to / from; }

          /* TODO: read idents from env */
          template <op O>
          parse::cell::cell apply_all(scope&, parse::cell::list const &cl)
          {
            auto const list(cl.data);
            expect::at_least_args(cl, 2);

            parse::cell::integer::type val
            {
              expect::type<parse::cell::type::integer>
              (
                *std::next(list.begin())
              ).data
            };
            for(auto &i : jtl::it::make_range(std::next(list.begin(), 2), list.end()))
            { val = apply<O>(val, expect::type<parse::cell::type::integer>(i).data); }

            return parse::cell::integer{ val };
          }
        }

        inline parse::cell::cell sum(scope &env, parse::cell::list const &cl)
        { return detail::apply_all<detail::op::add>(env, cl); }

        inline parse::cell::cell difference(scope &env, parse::cell::list const &cl)
        { return detail::apply_all<detail::op::sub>(env, cl); }

        inline parse::cell::cell product(scope &env, parse::cell::list const &cl)
        { return detail::apply_all<detail::op::mul>(env, cl); }

        inline parse::cell::cell quotient(scope &env, parse::cell::list const &cl)
        { return detail::apply_all<detail::op::div>(env, cl); }
      }
    }
  }
}
