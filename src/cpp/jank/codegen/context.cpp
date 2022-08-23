#include <iostream>

#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/codegen/context.hpp>

namespace jank::codegen
{
  namespace detail
  {
    void gen_constant(runtime::object_ptr const &o, std::ostream &oss)
    {
      if(o->as_nil())
      { oss << "jank::runtime::JANK_NIL"; }
      else if(auto d = o->as_boolean())
      { oss << "jank::runtime::make_box<jank::runtime::obj::boolean>(" << d->data << ")"; }
      else if(auto d = o->as_integer())
      { oss << "jank::runtime::make_box<jank::runtime::obj::integer>(" << d->data << ")"; }
      else if(auto d = o->as_real())
      { oss << "jank::runtime::make_box<jank::runtime::obj::real>(" << d->data << ")"; }
      else if(auto d = o->as_symbol())
      { oss << "jank::runtime::make_box<jank::runtime::obj::symbol>(\"" << d->ns << "\", \"" << d->name << "\")"; }
      else
      { std::cerr << "unimplemented constant codegen: " << *o << std::endl; }
    }
  }

  context::context
  (
    runtime::context &rt_ctx,
    analyze::context &an_ctx,
    analyze::processor::iterator const an_begin,
    analyze::processor::iterator const an_end,
    size_t const total_forms
  )
    : rt_ctx{ rt_ctx }, an_ctx{ an_ctx }
  {
    expressions.reserve(total_forms);
    for(auto it(an_begin); it != an_end; ++it)
    { expressions.emplace_back(std::move(it->expect_ok_move().unwrap())); }
  }

  void context::gen(analyze::expression const &ex, std::ostream &oss) const
  {
    boost::apply_visitor
    (
      [this, &oss](auto const &typed_ex)
      { gen(typed_ex, oss); },
      ex.data
    );
  }

  void context::gen(analyze::expr::def<analyze::expression> const &expr, std::ostream &oss) const
  {
    //std::cout << "gen def" << std::endl;
    auto const &var(an_ctx.find_lifted_var(expr.name).unwrap().get());
    oss << var.local_name.name << "->set_root(";
    //gen(expr.value, oss);
    oss << ");";
  }

  void context::gen(analyze::expr::var_deref<analyze::expression> const &expr, std::ostream &oss) const
  {
    //std::cout << "gen var deref" << std::endl;
    auto const &var(an_ctx.find_lifted_var(expr.qualified_name).unwrap().get());
    oss << var.local_name.name;
  }

  void context::gen(analyze::expr::call<analyze::expression> const &expr, std::ostream &oss) const
  {
    //std::cout << "gen call" << std::endl;
    gen(expr.source, oss);
    oss << "->call(";
    for(auto const &arg_expr : expr.arg_exprs)
    /* TODO: Comma separate. */
    { gen(arg_expr, oss); }
    oss << ");";
  }

  void context::gen(analyze::expr::primitive_literal<analyze::expression> const &expr, std::ostream &oss) const
  {
    //std::cout << "gen literal" << std::endl;
    auto const &constant(an_ctx.find_lifted_constant(expr.data).unwrap().get());
    oss << constant.local_name.name;
  }

  void context::gen(analyze::expr::vector<analyze::expression> const &, std::ostream &) const
  {
  }

  void context::gen(analyze::expr::local_reference<analyze::expression> const &, std::ostream &) const
  {
  }

  void context::gen(analyze::expr::function<analyze::expression> const &, std::ostream &) const
  {
  }

  std::string context::str() const
  {
    std::ostringstream oss;
    header_str(oss);
    body_str(oss);
    footer_str(oss);
    return oss.str();
  }

  void context::header_str(std::ostream &oss) const
  {
    auto const struct_name(an_ctx.unique_name());
    oss << "namespace jank::generated { struct " << struct_name.name << "{";

    for(auto const &v : an_ctx.tracked_refs.lifted_vars)
    { oss << "jank::runtime::var_ptr const " << v.second.local_name.name << ";"; }

    for(auto const &v : an_ctx.tracked_refs.lifted_constants)
    { oss << "jank::runtime::object_ptr const " << v.second.local_name.name << ";"; }

    /* TODO: Prevent name collision with rt_ctx. */
    oss << struct_name.name << "(jank::runtime::context &rt_ctx)";
    if(an_ctx.tracked_refs.lifted_vars.size() > 0 || an_ctx.tracked_refs.lifted_constants.size() > 0)
    { oss << " : "; }

    bool need_comma{};
    for(auto const &v : an_ctx.tracked_refs.lifted_vars)
    {
      oss << (need_comma ? "," : "") << v.second.local_name.name
           << "{ rt_ctx.intern_var("
           << "\"" << v.second.var_name->ns << "\""
           << ", \"" << v.second.var_name->name << "\""
           << ").expect_ok() }";
      need_comma = true;
    }

    for(auto const &v : an_ctx.tracked_refs.lifted_constants)
    {
      oss << (need_comma ? "," : "") << v.second.local_name.name << "{ ";
      detail::gen_constant(v.second.data, oss);
      oss << " }";
      need_comma = true;
    }

    oss << "{";
  }

  void context::body_str(std::ostream &oss) const
  {
    /* TODO: Put into fn with return type and return value of last expression. */
    for(auto const &expr : expressions)
    { gen(expr, oss); }
  }

  void context::footer_str(std::ostream &oss) const
  { oss << "}};}"; }
}
