#include <iostream>

#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/codegen/context.hpp>

namespace jank::codegen
{
  namespace detail
  {
    void gen_constant(runtime::object_ptr const &o, std::ostream &os)
    {
      if(o->as_nil())
      { os << "jank::runtime::JANK_NIL"; }
      else if(auto d = o->as_boolean())
      { os << "jank::runtime::make_box<jank::runtime::boolean>(" << d->data << ")"; }
      else if(auto d = o->as_integer())
      { os << "jank::runtime::make_box<jank::runtime::integer>(" << d->data << ")"; }
      else if(auto d = o->as_real())
      { os << "jank::runtime::make_box<jank::runtime::real>(" << d->data << ")"; }
      else if(auto d = o->as_symbol())
      { os << "jank::runtime::make_box<jank::runtime::symbol>(\"" << d->ns << "\", \"" << d->name << "\")"; }
      else
      { std::cerr << "unimplemented constant codegen: " << *o << std::endl; }
    }
  }

  context::context(runtime::context &rt_ctx, analyze::context &anal_ctx)
    : rt_ctx{ rt_ctx }, anal_ctx{ anal_ctx }
  { }

  void context::gen(analyze::expression const &ex)
  {
    boost::apply_visitor
    (
      [this](auto const &typed_ex)
      { gen(typed_ex); },
      ex.data
    );
  }

  void context::gen(analyze::expr::def<analyze::expression> const &expr)
  {
    //std::cout << "gen def" << std::endl;
    auto const &var(anal_ctx.find_lifted_var(expr.name).unwrap().get());
    oss << var.local_name.name << "->set_root(";
    gen(expr.value);
    oss << ");";
  }

  void context::gen(analyze::expr::var_deref<analyze::expression> const &)
  {
    //std::cout << "gen var deref" << std::endl;
  }

  void context::gen(analyze::expr::call<analyze::expression> const &)
  {
    //std::cout << "gen call" << std::endl;
  }

  void context::gen(analyze::expr::primitive_literal<analyze::expression> const &expr)
  {
    //std::cout << "gen literal" << std::endl;
    auto const &constant(anal_ctx.find_lifted_constant(expr.data).unwrap().get());
    oss << constant.local_name.name;
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

  std::string context::header_str() const
  {
    auto const struct_name(anal_ctx.unique_name());
    std::ostringstream hoss;
    hoss << "namespace jank::generated { struct " << struct_name.name << "{";

    for(auto const &v : anal_ctx.lifted_vars)
    { hoss << "jank::runtime::var_ptr const " << v.second.local_name.name << ";"; }

    for(auto const &v : anal_ctx.lifted_constants)
    { hoss << "jank::runtime::object_ptr const " << v.second.local_name.name << ";"; }

    /* TODO: Prevent name collision with rt_ctx. */
    hoss << struct_name.name << "(jank::runtime::context &rt_ctx)";
    if(anal_ctx.lifted_vars.size() > 0 || anal_ctx.lifted_constants.size() > 0)
    { hoss << " : "; }

    bool need_comma{};
    for(auto const &v : anal_ctx.lifted_vars)
    {
      hoss << (need_comma ? "," : "") << v.second.local_name.name
           << "{ rt_ctx.intern_var("
           << "\"" << v.second.var_name->ns << "\""
           << ", \"" << v.second.var_name->name << "\""
           << ") }";
      need_comma = true;
    }

    for(auto const &v : anal_ctx.lifted_constants)
    {
      hoss << (need_comma ? "," : "") << v.second.local_name.name << "{ ";
      detail::gen_constant(v.second.data, hoss);
      hoss << " }";
      need_comma = true;
    }

    hoss << "{";
    return hoss.str();
  }

  std::string context::body_str() const
  { return oss.str(); }

  std::string context::footer_str() const
  {
    std::ostringstream hoss;
    hoss << "}};}";
    return hoss.str();
  }
}
