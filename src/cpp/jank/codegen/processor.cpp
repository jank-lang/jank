#include <iostream>

#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/codegen/processor.hpp>

namespace jank::codegen
{
  namespace detail
  {
    void gen_constant(runtime::object_ptr const &o, std::ostream &oss)
    {
      if(o->as_nil())
      { oss << "jank::runtime::make_box<jank::runtime::obj::nil>()"; }
      else if(auto d = o->as_boolean())
      { oss << "jank::runtime::make_box<jank::runtime::obj::boolean>(" << d->data << ")"; }
      else if(auto d = o->as_integer())
      { oss << "jank::runtime::make_box<jank::runtime::obj::integer>(" << d->data << ")"; }
      else if(auto d = o->as_real())
      { oss << "jank::runtime::make_box<jank::runtime::obj::real>(" << d->data << ")"; }
      else if(auto d = o->as_symbol())
      { oss << "jank::runtime::make_box<jank::runtime::obj::symbol>(\"" << d->ns << "\", \"" << d->name << "\")"; }
      else if(auto d = o->as_keyword())
      { oss << "rt_ctx.intern_keyword(\"" << d->sym.ns << "\", \"" << d->sym.name << "\", " <<  d->resolved << ")"; }
      else
      { std::cerr << "unimplemented constant codegen: " << *o << std::endl; }
    }
  }

  processor::processor
  (
    runtime::context &rt_ctx,
    analyze::context &an_ctx,
    analyze::processor::iterator const an_begin,
    analyze::processor::iterator const an_end,
    size_t const total_forms
  ) : rt_ctx{ rt_ctx }, an_ctx{ an_ctx }
  {
    expressions.reserve(total_forms);
    for(auto it(an_begin); it != an_end; ++it)
    { expressions.emplace_back(std::move(it->expect_ok_move().unwrap())); }
    struct_name = an_ctx.unique_name();
  }

  void processor::gen(analyze::expression const &ex, std::ostream &oss, bool const is_statement) const
  {
    boost::apply_visitor
    (
      [this, &oss, is_statement](auto const &typed_ex)
      { gen(typed_ex, oss, is_statement); },
      ex.data
    );
  }

  void processor::gen(analyze::expr::def<analyze::expression> const &expr, std::ostream &oss, bool const is_statement) const
  {
    /* Forward declarations don't need codegen. */
    if(expr.value.is_none())
    { return; }

    auto const &var(an_ctx.find_lifted_var(expr.name).unwrap().get());
    oss << var.local_name.name << "->set_root(";
    gen(*expr.value.unwrap(), oss, false);
    oss << ")";
    if(is_statement)
    { oss << ";"; }
  }

  void processor::gen(analyze::expr::var_deref<analyze::expression> const &expr, std::ostream &oss, bool const) const
  {
    auto const &var(an_ctx.find_lifted_var(expr.qualified_name).unwrap().get());
    oss << var.local_name.name << "->get_root()";
  }

  void processor::gen(analyze::expr::call<analyze::expression> const &expr, std::ostream &oss, bool const is_statement) const
  {
    gen(expr.source, oss, false);
    oss << "->as_callable()->call(";
    bool need_comma{};
    for(auto it = expr.arg_exprs.begin(); it != expr.arg_exprs.end(); ++it)
    {
      if(need_comma)
      { oss << ", "; }
      gen(*it, oss, false);
      need_comma = true;
    }
    oss << ")";
    if(is_statement)
    { oss << ";"; }
  }

  void processor::gen(analyze::expr::primitive_literal<analyze::expression> const &expr, std::ostream &oss, bool const) const
  {
    auto const &constant(an_ctx.find_lifted_constant(expr.data).unwrap().get());
    oss << constant.local_name.name;
  }

  void processor::gen(analyze::expr::vector<analyze::expression> const &expr, std::ostream &oss, bool const is_statement) const
  {
    assert(!is_statement);
    oss << "jank::runtime::make_box<jank::runtime::obj::vector>(";
    for(auto it(expr.data_exprs.begin()); it != expr.data_exprs.end();)
    {
      gen(*it, oss, false);
      if(++it != expr.data_exprs.end())
      { oss << ", "; }
    }
    oss << ")";
  }

  void processor::gen(analyze::expr::map<analyze::expression> const &expr, std::ostream &oss, bool const is_statement) const
  {
    assert(!is_statement);
    oss << "jank::runtime::make_box<jank::runtime::obj::map>(";
    oss << "jank::runtime::obj::map::variadic_tag{}, ";
    for(auto it(expr.data_exprs.begin()); it != expr.data_exprs.end();)
    {
      gen(it->first, oss, false);
      oss << ", ";
      gen(it->second, oss, false);
      if(++it != expr.data_exprs.end())
      { oss << ", "; }
    }
    oss << ")";
  }

  void processor::gen(analyze::expr::local_reference<analyze::expression> const &, std::ostream &, bool const) const
  {
  }

  void processor::gen(analyze::expr::function<analyze::expression> const &, std::ostream &, bool const) const
  {
  }

  std::string processor::str() const
  {
    std::ostringstream oss;
    oss << std::boolalpha;
    header_str(oss);
    body_str(oss);
    footer_str(oss);
    return oss.str();
  }

  void processor::header_str(std::ostream &oss) const
  {
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

  void processor::body_str(std::ostream &oss) const
  {
    /* TODO: Put into fn with return type and return value of last expression. */
    for(auto const &expr : expressions)
    { gen(expr, oss, true); }
  }

  void processor::footer_str(std::ostream &oss) const
  {
    oss << "}};} ";
    oss << "namespace { jank::generated::"
        << struct_name.name
        << " tmp{ "
        << "*reinterpret_cast<jank::runtime::context*>(" << std::hex << &rt_ctx << std::dec << ")"
        << " }; "
        << "}";
  }
}
