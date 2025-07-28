#include <jank/analyze/pass/optimize.hpp>
#include <jank/analyze/pass/strip_source_meta.hpp>
#include <jank/profile/time.hpp>

namespace jank::analyze::pass
{
  /* This is the general entry point to run multi-pass optimizations on an AST.
   * There is a default set of passes which are used and other passes may be
   * enabled/disabled via CLI flags.
   *
   * The AST is modified in place and the returned expression may not be the
   * same as the input expression. */
  expression_ref optimize(expression_ref expr)
  {
    profile::timer const timer{ "optimize ast" };

    expr = strip_source_meta(expr);

    /* TODO: Port force_boxed to use this system. */

    return expr;
  }
}
