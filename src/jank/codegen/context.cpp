#include <iostream>

#include <jank/runtime/context.hpp>
#include <jank/codegen/context.hpp>

namespace jank::codegen
{
  void context::gen(analyze::expression const &ex)
  {
    boost::apply_visitor
    (
      [this](auto const &typed_ex)
      { gen(typed_ex); },
      ex.data
    );
  }

  void context::gen(analyze::expr::def<analyze::expression> const &)
  {
  }

  void context::gen(analyze::expr::var_deref<analyze::expression> const &)
  {
  }

  void context::gen(analyze::expr::call<analyze::expression> const &)
  {
  }

  void context::gen(analyze::expr::primitive_literal<analyze::expression> const &)
  {
  }

  void context::gen(analyze::expr::vector<analyze::expression> const &)
  {
  }

  void context::gen(analyze::expr::local_reference<analyze::expression> const &)
  {
  }

  void context::gen(analyze::expr::function<analyze::expression> const &)
  {
  }

  std::string context::data() const
  { return oss.str(); }
}
