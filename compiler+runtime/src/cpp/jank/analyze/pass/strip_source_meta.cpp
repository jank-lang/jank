#include <jank/analyze/pass/strip_source_meta.hpp>
#include <jank/analyze/pass/walk.hpp>
#include <jank/analyze/visit.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::analyze::pass
{
  /* We add a lot of source info to parsed object meta which we don't need at runtime. This
   * optimization pass strips the meta where possible.
   *
   * Vars are not currently stripped of their meta, but they could be. */
  expression_ref strip_source_meta(expression_ref expr)
  {
    /* TODO: walk_data or walk_meta? */
    postwalk(expr, [&](expression_ref const e) {
      expr = visit_expr(
        [](auto const typed_e) -> expression_ref {
          using T = typename decltype(typed_e)::value_type;

          if constexpr(jtl::is_any_same<T, expr::list, expr::vector, expr::set, expr::map>)
          {
            if(typed_e->meta.is_some())
            {
              typed_e->meta = runtime::strip_source_from_meta(typed_e->meta);
            }
          }
          else if constexpr(jtl::is_same<T, expr::primitive_literal>)
          {
            runtime::visit_object(
              [](auto const typed_obj) {
                using O = typename decltype(typed_obj)::value_type;

                if constexpr(runtime::behavior::metadatable<O>)
                {
                  runtime::reset_meta(typed_obj,
                                      runtime::strip_source_from_meta(typed_obj->get_meta()));
                }
              },
              typed_e->data);
          }

          return typed_e;
        },
        e);
    });
    return expr;
  }
}
