#include <iostream>

#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/codegen/processor.hpp>

namespace jank::codegen
{
  namespace detail
  {
    void gen_constant(runtime::object_ptr const &, std::ostream &)
    { }
    void gen_constant(runtime::object_ptr const &o, fmt::memory_buffer &buffer)
    {
      auto inserter(std::back_inserter(buffer));
      if(o->as_nil())
      { format_to(inserter, "jank::runtime::make_box<jank::runtime::obj::nil>()"); }
      else if(auto const * const d = o->as_boolean())
      { format_to(inserter, "jank::runtime::make_box<jank::runtime::obj::boolean>({})", d->data); }
      else if(auto const * const d = o->as_integer())
      { format_to(inserter, "jank::runtime::make_box<jank::runtime::obj::integer>({})", d->data); }
      else if(auto const * const d = o->as_real())
      { format_to(inserter, "jank::runtime::make_box<jank::runtime::obj::real>({})", d->data); }
      else if(auto const * const d = o->as_symbol())
      {
        format_to
        (
          inserter,
          R"(jank::runtime::make_box<jank::runtime::obj::symbol>("{}", "{}"))",
          d->ns,
          d->name
        );
      }
      else if(auto const * const d = o->as_keyword())
      {
        format_to
        (
          inserter,
          R"(rt_ctx.intern_keyword("{}", "{}", {}))",
          d->sym.ns,
          d->sym.name,
          d->resolved
        );
      }
      else
      { std::cerr << "unimplemented constant codegen: " << *o << std::endl; }
    }
  }

  processor::processor
  (
    runtime::context &rt_ctx,
    analyze::context &an_ctx,
    analyze::processor::iterator const &an_begin,
    analyze::processor::iterator const &an_end,
    size_t const total_forms
  ) : rt_ctx{ rt_ctx }, an_ctx{ an_ctx }
  {
    expressions.reserve(total_forms);
    for(auto it(an_begin); it != an_end; ++it)
    { expressions.emplace_back(std::move(it->expect_ok_move().unwrap())); }
    struct_name = analyze::context::unique_name();
  }

  void processor::gen(analyze::expression const &ex, bool const is_statement)
  {
    boost::apply_visitor
    (
      [this, is_statement](auto const &typed_ex)
      { gen(typed_ex, is_statement); },
      ex.data
    );
  }

  void processor::gen(analyze::expr::def<analyze::expression> const &expr, bool const is_statement)
  {
    /* Forward declarations don't need codegen. */
    if(expr.value.is_none())
    { return; }

    auto inserter(std::back_inserter(body_buffer));
    auto const &var(an_ctx.find_lifted_var(expr.name).unwrap().get());
    format_to(inserter, "{}->set_root(", var.local_name.name);
    gen(*expr.value.unwrap(), false);
    format_to(inserter, "){}", (is_statement ? ";" : ""));
  }

  void processor::gen(analyze::expr::var_deref<analyze::expression> const &expr, bool const is_statement)
  {
    auto inserter(std::back_inserter(body_buffer));
    auto const &var(an_ctx.find_lifted_var(expr.qualified_name).unwrap().get());
    format_to
    (
      inserter,
      "{}->get_root(){}",
      var.local_name.name,
      (is_statement ? ";" : "")
    );
  }

  void processor::gen(analyze::expr::call<analyze::expression> const &expr, bool const is_statement)
  {
    auto inserter(std::back_inserter(body_buffer));
    gen(expr.source, false);
    format_to(inserter, "->as_callable()->call(");
    bool need_comma{};
    for(auto const &arg_expr : expr.arg_exprs)
    {
      if(need_comma)
      { format_to(inserter, ", "); }
      gen(arg_expr, false);
      need_comma = true;
    }
    format_to(inserter, ")");
    if(is_statement)
    { format_to(inserter, ";"); }
  }

  void processor::gen(analyze::expr::primitive_literal<analyze::expression> const &expr, bool const is_statement)
  {
    auto inserter(std::back_inserter(body_buffer));
    auto const &constant(an_ctx.find_lifted_constant(expr.data).unwrap().get());
    format_to
    (
      inserter,
      "{}{}",
      constant.local_name.name,
      (is_statement ? ";" : "")
    );
  }

  void processor::gen(analyze::expr::vector<analyze::expression> const &expr, bool const is_statement)
  {
    assert(!is_statement);
    auto inserter(std::back_inserter(body_buffer));
    format_to(inserter, "jank::runtime::make_box<jank::runtime::obj::vector>(");
    for(auto it(expr.data_exprs.begin()); it != expr.data_exprs.end();)
    {
      gen(*it, false);
      if(++it != expr.data_exprs.end())
      { format_to(inserter, ", "); }
    }
    format_to(inserter, "){}", (is_statement ? ";" : ""));
  }

  void processor::gen(analyze::expr::map<analyze::expression> const &expr, bool const is_statement)
  {
    assert(!is_statement);
    auto inserter(std::back_inserter(body_buffer));
    format_to(inserter, "jank::runtime::make_box<jank::runtime::obj::map>(std::in_place, ");
    for(auto it(expr.data_exprs.begin()); it != expr.data_exprs.end();)
    {
      gen(it->first, false);
      format_to(inserter, ", ");
      gen(it->second, false);
      if(++it != expr.data_exprs.end())
      { format_to(inserter, ", "); }
    }
    format_to(inserter, "){}", (is_statement ? ";" : ""));
  }

  void processor::gen(analyze::expr::local_reference<analyze::expression> const &, bool const)
  {
  }

  void processor::gen(analyze::expr::function<analyze::expression> const &, bool const)
  {
  }

  std::string processor::declaration_str()
  {
    if(!generated_declaration)
    {
      build_header();
      build_body();
      build_footer();
      generated_declaration = true;
    }

    std::string ret;
    ret.reserve(header_buffer.size() + body_buffer.size() + footer_buffer.size());
    ret += std::string_view{ header_buffer.data(), header_buffer.size() };
    ret += std::string_view{ body_buffer.data(), body_buffer.size() };
    ret += std::string_view{ footer_buffer.data(), footer_buffer.size() };
    return ret;
  }

  void processor::build_header()
  {
    auto inserter(std::back_inserter(header_buffer));
    format_to
    (
      inserter,
      "namespace jank::generated {{ struct {0} {{", struct_name.name
    );

    for(auto const &v : an_ctx.tracked_refs.lifted_vars)
    {
      format_to
      (
        inserter,
        "jank::runtime::var_ptr const {0};", v.second.local_name.name
      );
    }

    for(auto const &v : an_ctx.tracked_refs.lifted_constants)
    {
      format_to
      (
        inserter,
        "jank::runtime::object_ptr const {0};", v.second.local_name.name
      );
    }

    /* TODO: Prevent name collision with rt_ctx. */
    format_to
    (
      inserter,
      "{0}(jank::runtime::context &rt_ctx)", struct_name.name
    );
    if(!an_ctx.tracked_refs.lifted_vars.empty() || !an_ctx.tracked_refs.lifted_constants.empty())
    { format_to(inserter, " : "); }

    bool need_comma{};
    for(auto const &v : an_ctx.tracked_refs.lifted_vars)
    {
      format_to
      (
        inserter,
        R"({0}{1}{{ rt_ctx.intern_var("{2}", "{3}").expect_ok() }})",
        (need_comma ? "," : ""),
        v.second.local_name.name,
        v.second.var_name->ns,
        v.second.var_name->name
      );
      need_comma = true;
    }

    for(auto const &v : an_ctx.tracked_refs.lifted_constants)
    {
      format_to
      (
        inserter,
        "{0}{1}{{",
        (need_comma ? "," : ""),
        v.second.local_name.name
      );
      detail::gen_constant(v.second.data, header_buffer);
      format_to(inserter, "}}");
      need_comma = true;
    }

    format_to(inserter, "{{ }}");
  }

  void processor::build_body()
  {
    auto inserter(std::back_inserter(body_buffer));
    format_to(inserter, "jank::runtime::object_ptr call() {{");

    for(auto const * it(expressions.begin()); it != expressions.end(); ++it)
    {
      if(it + 1 == expressions.end())
      { format_to(inserter, "return "); }
      gen(*it, true);
    }

    format_to(inserter, "}}");
  }

  void processor::build_footer()
  {
    auto inserter(std::back_inserter(footer_buffer));
    format_to(inserter, "}};}}");
  }

  std::string processor::expression_str()
  {
    if(!generated_expression)
    {
      auto inserter(std::back_inserter(expression_buffer));
      auto tmp_name(analyze::context::unique_name());
      format_to
      (
        inserter,
        R"(
          jank::generated::{0} {1}{{ *reinterpret_cast<jank::runtime::context*>({2}) }};
          {1}.call();
        )",
        struct_name.name,
        tmp_name.name,
        fmt::ptr(&rt_ctx)
      );
    }
    return { expression_buffer.data(), expression_buffer.size() };
  }
}
