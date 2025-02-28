#include <jank/analyze/step/force_boxed.hpp>
#include <jank/analyze/expression.hpp>
#include <jank/analyze/visit.hpp>

namespace jank::analyze::step
{
  /* TODO: This is more a walk_tail. A general walk would apply to all nodes. */
  /* TODO: This needs to be recursive to work properly. */
  template <typename F>
  static void walk(expression_ptr const expr, F const &f)
  {
    visit_expr(
      [&](auto const typed_expr) {
        using T = std::decay_t<decltype(typed_expr)>;

        if constexpr(std::same_as<T, expr::if_>)
        {
          visit_expr(f, typed_expr->then);
          if(typed_expr->else_.is_some())
          {
            visit_expr(f, typed_expr->else_.unwrap());
          }
        }
        else if constexpr(std::same_as<T, expr::do_>)
        {
          if(!typed_expr->values.empty())
          {
            visit_expr(f, typed_expr->values.back());
          }
        }
        else if constexpr(std::same_as<T, expr::let>)
        {
          if(!typed_expr->body.values.empty())
          {
            visit_expr(f, typed_expr->body.values.back());
          }
        }
        else if constexpr(std::same_as<T, expr::try_>)
        {
          if(!typed_expr->body.values.empty())
          {
            visit_expr(f, typed_expr->body.values.back());
          }
          if(typed_expr->catch_body && !typed_expr->catch_body.unwrap().body.values.empty())
          {
            visit_expr(f, typed_expr->catch_body.unwrap().body.values.back());
          }
        }
        /* TODO: Case */
        else
        {
          f(typed_expr);
        }
      },
      expr);
  }

  /* Mutated in place. */
  void force_boxed(expr::do_ptr const do_)
  {
    if(do_->needs_box)
    {
      return;
    }

    do_->needs_box = true;

    if(!do_->values.empty())
    {
      auto &last(do_->values.back());
      walk(last, [](auto &typed_expr) {
        //using T = std::decay_t<decltype(typed_expr)>;
        typed_expr->needs_box = true;
      });
    }
  }
}
