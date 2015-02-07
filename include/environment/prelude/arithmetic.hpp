#pragma once

#include <jtl/iterator/range.hpp>

#include "cell.hpp"
#include "environment/detail/expect.hpp"

inline cell add(cell_list const &cl)
{
  auto const list(cl.data);
  detail::expect_at_least_args(cl, 2);

  cell_int::type val{};
  for(auto &i : jtl::it::make_range(std::next(list.begin()), list.end()))
  { val += detail::expect_type<cell_type::integer>(i).data; }

  return cell_int{ val };
}
