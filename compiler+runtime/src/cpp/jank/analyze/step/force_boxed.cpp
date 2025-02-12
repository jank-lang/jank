#include <jank/analyze/step/force_boxed.hpp>
#include <jank/analyze/expression.hpp>

namespace jank::analyze::step
{
  /* TODO: This is more a walk_tail. A general walk would apply to all nodes. */
  /* TODO: This needs to be recursive to work properly. */
  template <typename F>
  static void walk(expression_ptr const expr, F const &f)
  {
    boost::apply_visitor(
      [&](auto &typed_expr) {
        using T = std::decay_t<decltype(typed_expr)>;

        if constexpr(std::same_as<T, expr::if_<expression>>)
        {
          boost::apply_visitor(f, typed_expr.then->data);
          if(typed_expr.else_.is_some())
          {
            boost::apply_visitor(f, typed_expr.else_.unwrap()->data);
          }
        }
        else if constexpr(std::same_as<T, expr::do_<expression>>)
        {
          if(!typed_expr.values.empty())
          {
            boost::apply_visitor(f, typed_expr.values.back()->data);
          }
        }
        else if constexpr(std::same_as<T, expr::let<expression>>
                          || std::same_as<T, expr::letfn<expression>>)
        {
          if(!typed_expr.body.values.empty())
          {
            boost::apply_visitor(f, typed_expr.body.values.back()->data);
          }
        }
        else if constexpr(std::same_as<T, expr::try_<expression>>)
        {
          if(!typed_expr.body.values.empty())
          {
            boost::apply_visitor(f, typed_expr.body.values.back()->data);
          }
          if(typed_expr.catch_body && !typed_expr.catch_body.unwrap().body.values.empty())
          {
            boost::apply_visitor(f, typed_expr.catch_body.unwrap().body.values.back()->data);
          }
        }
        else
        {
          f(typed_expr);
        }
      },
      expr->data);
  }

  expr::do_<expression> force_boxed(expr::do_<expression> &&do_)
  {
    if(do_.needs_box)
    {
      return std::move(do_);
    }

    do_.needs_box = true;

    if(!do_.values.empty())
    {
      auto &last(do_.values.back());
      walk(last, [](auto &typed_expr) {
        //using T = std::decay_t<decltype(typed_expr)>;
        typed_expr.needs_box = true;
      });
    }

    return std::move(do_);
  }
}
