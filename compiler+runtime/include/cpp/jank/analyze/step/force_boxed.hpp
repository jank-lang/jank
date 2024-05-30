#pragma once

namespace jank::analyze::step
{
  expr::do_<expression> force_boxed(expr::do_<expression> &&do_);
}
