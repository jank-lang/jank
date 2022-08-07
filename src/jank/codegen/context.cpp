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
      { oss << "jank::runtime::make_box<jank::runtime::boolean>(" << d->data << ")"; }
      else if(auto d = o->as_integer())
      { oss << "jank::runtime::make_box<jank::runtime::integer>(" << d->data << ")"; }
      else if(auto d = o->as_real())
      { oss << "jank::runtime::make_box<jank::runtime::real>(" << d->data << ")"; }
      else if(auto d = o->as_symbol())
      { oss << "jank::runtime::make_box<jank::runtime::symbol>(\"" << d->ns << "\", \"" << d->name << "\")"; }
      else
      { std::cerr << "unimplemented constant codegen: " << *o << std::endl; }
    }
  }

  context::context(runtime::context &rt_ctx, analyze::context &an_ctx)
    : rt_ctx{ rt_ctx }, an_ctx{ an_ctx }
  { }

  void context::gen(analyze::expression const &ex, std::ostream &oss)
  {
    boost::apply_visitor
    (
      [this, &oss](auto const &typed_ex)
      { gen(typed_ex, oss); },
      ex.data
    );
  }

  void context::gen(analyze::expr::def<analyze::expression> const &expr, std::ostream &oss)
  {
    //std::cout << "gen def" << std::endl;
    auto const &var(an_ctx.find_lifted_var(expr.name).unwrap().get());
    oss << var.local_name.name << "->set_root(";
    gen(expr.value, oss);
    oss << ");";
  }

  void context::gen(analyze::expr::var_deref<analyze::expression> const &expr, std::ostream &oss)
  {
    //std::cout << "gen var deref" << std::endl;
    auto const &var(an_ctx.find_lifted_var(expr.var->name).unwrap().get());
    oss << var.local_name.name;
  }

  void context::gen(analyze::expr::call<analyze::expression> const &, std::ostream &)
  {
    //std::cout << "gen call" << std::endl;
  }

  void context::gen(analyze::expr::primitive_literal<analyze::expression> const &expr, std::ostream &oss)
  {
    //std::cout << "gen literal" << std::endl;
    auto const &constant(an_ctx.find_lifted_constant(expr.data).unwrap().get());
    oss << constant.local_name.name;
  }

  void context::gen(analyze::expr::vector<analyze::expression> const &, std::ostream &)
  {
  }

  void context::gen(analyze::expr::local_reference<analyze::expression> const &, std::ostream &)
  {
  }

  void context::gen(analyze::expr::function<analyze::expression> const &, std::ostream &)
  {
  }

  std::string context::header_str() const
  {
    auto const struct_name(an_ctx.unique_name());
    std::ostringstream oss;
    oss << "namespace jank::generated { struct " << struct_name.name << "{";

    for(auto const &v : an_ctx.lifted_vars)
    { oss << "jank::runtime::var_ptr const " << v.second.local_name.name << ";"; }

    for(auto const &v : an_ctx.lifted_constants)
    { oss << "jank::runtime::object_ptr const " << v.second.local_name.name << ";"; }

    /* TODO: Prevent name collision with rt_ctx. */
    oss << struct_name.name << "(jank::runtime::context &rt_ctx)";
    if(an_ctx.lifted_vars.size() > 0 || an_ctx.lifted_constants.size() > 0)
    { oss << " : "; }

    bool need_comma{};
    for(auto const &v : an_ctx.lifted_vars)
    {
      oss << (need_comma ? "," : "") << v.second.local_name.name
           << "{ rt_ctx.intern_var("
           << "\"" << v.second.var_name->ns << "\""
           << ", \"" << v.second.var_name->name << "\""
           << ") }";
      need_comma = true;
    }

    for(auto const &v : an_ctx.lifted_constants)
    {
      oss << (need_comma ? "," : "") << v.second.local_name.name << "{ ";
      detail::gen_constant(v.second.data, oss);
      oss << " }";
      need_comma = true;
    }

    oss << "{";
    return oss.str();
  }

  std::string context::body_str() const
  {
    std::ostringstream oss;
    // TODO: gen
    return oss.str();
  }

  std::string context::footer_str() const
  {
    std::ostringstream oss;
    oss << "}};}";
    return oss.str();
  }
}
