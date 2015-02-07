#pragma once

#include <jtl/iterator/range.hpp>

#include "cell.hpp"
#include "environment/detail/expect.hpp"

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
  auto apply(cell_int::type const to, cell_int::type const from);
  template <>
  inline auto apply<op::add>(cell_int::type const to, cell_int::type const from)
  { return to + from; }
  template <>
  inline auto apply<op::sub>(cell_int::type const to, cell_int::type const from)
  { return to - from; }
  template <>
  inline auto apply<op::mul>(cell_int::type const to, cell_int::type const from)
  { return to * from; }
  template <>
  inline auto apply<op::div>(cell_int::type const to, cell_int::type const from)
  { return to / from; }

  template <op O>
  cell apply_all(cell_list const &cl)
  {
    auto const list(cl.data);
    detail::expect_at_least_args(cl, 2);

    cell_int::type val
    {
      detail::expect_type<cell_type::integer>
      (
        *std::next(list.begin())
      ).data
    };
    for(auto &i : jtl::it::make_range(std::next(list.begin(), 2), list.end()))
    { val = apply<O>(val, detail::expect_type<cell_type::integer>(i).data); }

    return cell_int{ val };
  }
}

inline cell sum(cell_list const &cl)
{ return detail::apply_all<detail::op::add>(cl); }

inline cell difference(cell_list const &cl)
{ return detail::apply_all<detail::op::sub>(cl); }

inline cell product(cell_list const &cl)
{ return detail::apply_all<detail::op::mul>(cl); }

inline cell quotient(cell_list const &cl)
{ return detail::apply_all<detail::op::div>(cl); }
