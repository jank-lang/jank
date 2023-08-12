
namespace jank::analyze::step
{
  template <typename F>
  void walk(expression_ptr const expr, F const &f)
  {
    boost::apply_visitor
    (
      [&](auto &typed_expr)
      {
        using T = std::decay_t<decltype(typed_expr)>;

        if constexpr(std::same_as<T, expr::if_<expression>>)
        {
          boost::apply_visitor(f, typed_expr.then->data);
          if(typed_expr.else_.is_some())
          { boost::apply_visitor(f, typed_expr.else_.unwrap()->data); }
        }
        else if constexpr(std::same_as<T, expr::do_<expression>>)
        {
          if(!typed_expr.body.empty())
          { boost::apply_visitor(f, typed_expr.body.back()->data); }
        }
        else if constexpr(std::same_as<T, expr::let<expression>>)
        {
          if(!typed_expr.body.body.empty())
          { boost::apply_visitor(f, typed_expr.body.body.back()->data); }
        }
        else
        { f(typed_expr); }
      },
      expr->data
    );
  }

  expr::do_<expression> force_boxed(expr::do_<expression> &&do_)
  {
    if(do_.needs_box)
    { return std::move(do_); }

    do_.needs_box = true;

    if(!do_.body.empty())
    {
      auto &last(do_.body.back());
      walk
      (
        last,
        [](auto &typed_expr)
        {
          //using T = std::decay_t<decltype(typed_expr)>;
          typed_expr.needs_box = true;
        }
      );
    }

    return std::move(do_);
  }
}
