#include <jank/analyze/pass/walk.hpp>

namespace jank::analyze::pass
{
  void postwalk(expression_ref const expr, std::function<void(expression_ref)> const &f)
  {
    expr->walk([&](expression_ref const e) { postwalk(e, f); });
    f(expr);
  }

  void prewalk(expression_ref const expr, std::function<void(expression_ref)> const &f)
  {
    f(expr);
    expr->walk([&](expression_ref const e) { prewalk(e, f); });
  }
}
