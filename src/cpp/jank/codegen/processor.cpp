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
    analyze::expression const &expr
  )
    : rt_ctx{ rt_ctx }, an_ctx{ an_ctx },
      root_expression{ boost::get<analyze::expr::function<analyze::expression>>(expr.data) },
      struct_name{ analyze::context::unique_name() }
  { }

  /* TODO: Use source names as much as possible and sanitize all of them. */
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
    auto inserter(std::back_inserter(body_buffer));
    auto const &var(an_ctx.find_lifted_var(expr.name).unwrap().get());

    /* Forward declarations just intern the var and evaluate to it. */
    if(expr.value.is_none())
    {
      format_to(inserter, "{}{}", var.local_name.name, (is_statement ? ";" : ""));
      return;
    }

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
    gen(*expr.source, false);
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

  void processor::gen(analyze::expr::local_reference<analyze::expression> const &expr, bool const is_statement)
  {
    auto inserter(std::back_inserter(body_buffer));
    format_to(inserter, "{}{}", expr.name->to_string(), (is_statement ? ";" : ""));
  }

  void processor::gen(analyze::expr::function<analyze::expression> const &expr, bool const is_statement)
  {
    /* Since each codegen proc handles one callable struct, we create a new one for this fn. */
    processor prc{ rt_ctx, an_ctx, expr };

    auto header_inserter(std::back_inserter(header_buffer));
    auto body_inserter(std::back_inserter(body_buffer));
    format_to(header_inserter, "{}", prc.declaration_str());
    format_to(body_inserter, "{}{}", prc.expression_str(false), (is_statement ? ";" : ""));
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
      R"(
        struct {0}
          : jank::runtime::object,
            jank::runtime::pool_item_base<{0}>,
            jank::runtime::behavior::callable
        {{
      )",
      struct_name.name
    );

    format_to
    (
      inserter,
      R"(
        jank::runtime::detail::boolean_type equal(object const &rhs) const override
        {{ return this == &rhs; }}
        jank::runtime::detail::string_type to_string() const override
        {{ return "jit function"; }}
        jank::runtime::detail::integer_type to_hash() const override
        {{ return reinterpret_cast<jank::runtime::detail::integer_type>(this); }}
        jank::runtime::behavior::callable const* as_callable() const override
        {{ return this; }}
      )"
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

    for(auto const &v : root_expression.frame.captures)
    {
      format_to
      (
        inserter,
        "jank::runtime::object_ptr const {0};", v.first->name
      );
    }

    /* TODO: Prevent name collision with rt_ctx. */
    format_to
    (
      inserter,
      "{0}(jank::runtime::context &rt_ctx", struct_name.name
    );

    for(auto const &v : root_expression.frame.captures)
    {
      format_to
      (
        inserter,
        ", jank::runtime::object_ptr const &{0}", v.first->name
      );
    }

    format_to(inserter, ")");

    if(!an_ctx.tracked_refs.lifted_vars.empty() || !an_ctx.tracked_refs.lifted_constants.empty())
    { format_to(inserter, " : "); }

    bool need_member_init_comma{};
    for(auto const &v : an_ctx.tracked_refs.lifted_vars)
    {
      format_to
      (
        inserter,
        R"({0}{1}{{ rt_ctx.intern_var("{2}", "{3}").expect_ok() }})",
        (need_member_init_comma ? "," : ""),
        v.second.local_name.name,
        v.second.var_name->ns,
        v.second.var_name->name
      );
      need_member_init_comma = true;
    }

    for(auto const &v : an_ctx.tracked_refs.lifted_constants)
    {
      format_to
      (
        inserter,
        "{0}{1}{{",
        (need_member_init_comma ? "," : ""),
        v.second.local_name.name
      );
      detail::gen_constant(v.second.data, header_buffer);
      format_to(inserter, "}}");
      need_member_init_comma = true;
    }

    for(auto const &v : root_expression.frame.captures)
    {
      format_to
      (
        inserter,
        "{0}{1}{{ {1} }}",
        (need_member_init_comma ? "," : ""),
        v.first->name
      );
      need_member_init_comma = true;
    }

    format_to(inserter, "{{ }}");
  }

  void processor::build_body()
  {
    auto inserter(std::back_inserter(body_buffer));
    format_to(inserter, "jank::runtime::object_ptr call(");
    bool param_comma{};
    for(auto const &param : root_expression.params)
    {
      format_to(inserter, "{} jank::runtime::object_ptr const &{}", (param_comma ? ", " : ""), param->to_string());
      param_comma = true;
    }
    format_to(inserter, ") const override {{");

    for(auto it(root_expression.body.body.begin()); it != root_expression.body.body.end();)
    {
      auto const &form(*it);
      if(++it == root_expression.body.body.end())
      { format_to(inserter, "return "); }
      gen(form, true);
    }

    format_to(inserter, "}}");
  }

  void processor::build_footer()
  {
    auto inserter(std::back_inserter(footer_buffer));
    format_to(inserter, "}};");
  }

  /* TODO: Something better than a default arg. */
  std::string processor::expression_str(bool const auto_call)
  {
    if(!generated_expression)
    {
      auto inserter(std::back_inserter(expression_buffer));

      if(auto_call)
      {
        /* TODO: There's a Cling bug here which prevents us from returning the fn object itself,
         * to be called in non-JIT code. If we call it here and return the result, it works fine. */
        auto tmp_name(analyze::context::unique_name());
        format_to
        (
          inserter,
          R"(
            {0} {1}{{ *reinterpret_cast<jank::runtime::context*>({2})
          )",
          struct_name.name,
          tmp_name.name,
          fmt::ptr(&rt_ctx)
        );

        for(auto const &v : root_expression.frame.captures)
        { format_to(inserter, ", {0}", v.first->name); }

        format_to(inserter, "}};");

        format_to
        (
          inserter,
          "{1}.call();",
          struct_name.name,
          tmp_name.name,
          fmt::ptr(&rt_ctx)
        );
      }
      else
      {
        format_to
        (
          inserter,
          "jank::runtime::make_box<{0}>(std::ref(*reinterpret_cast<jank::runtime::context*>({1}))",
          struct_name.name,
          fmt::ptr(&rt_ctx)
        );

        for(auto const &v : root_expression.frame.captures)
        { format_to(inserter, ", {0}", v.first->name); }

        format_to(inserter, ")");
      }

      generated_expression = true;
    }
    return { expression_buffer.data(), expression_buffer.size() };
  }
}
