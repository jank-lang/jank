#include <iostream>

#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/util.hpp>
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
    auto const &var(expr.frame->find_lifted_var(expr.name).unwrap().get());

    /* Forward declarations just intern the var and evaluate to it. */
    if(expr.value.is_none())
    {
      format_to
      (
        inserter,
        "{}{}",
        runtime::munge(var.native_name.name),
        (is_statement ? ";" : "")
      );
      return;
    }

    format_to(inserter, "{}->set_root(", runtime::munge(var.native_name.name));
    gen(*expr.value.unwrap(), false);
    format_to(inserter, "){}", (is_statement ? ";" : ""));
  }

  void processor::gen(analyze::expr::var_deref<analyze::expression> const &expr, bool const is_statement)
  {
    auto inserter(std::back_inserter(body_buffer));
    auto const &var(expr.frame->find_lifted_var(expr.qualified_name).unwrap().get());
    format_to
    (
      inserter,
      "{}->get_root(){}",
      runtime::munge(var.native_name.name),
      (is_statement ? ";" : "")
    );
  }

  void processor::gen(analyze::expr::call<analyze::expression> const &expr, bool const is_statement)
  {
    auto inserter(std::back_inserter(body_buffer));

    if(expr.required_packed_args.is_none())
    {
      format_to(inserter, "jank::runtime::dynamic_call(");
      gen(*expr.source, false);
      for(auto const &arg_expr : expr.arg_exprs)
      {
        format_to(inserter, ", ");
        gen(arg_expr, false);
      }
      format_to(inserter, ")");
    }
    else
    {
      gen(*expr.source, false);
      format_to(inserter, "->as_callable()->call(");
      bool need_comma{};
      for(size_t i{}; i < expr.arg_exprs.size() - expr.required_packed_args.unwrap(); ++i)
      {
        if(need_comma)
        { format_to(inserter, ", "); }
        gen(expr.arg_exprs[i], false);
        need_comma = true;
      }

      if(expr.required_packed_args.unwrap() > 0)
      {
        if(need_comma)
        { format_to(inserter, ", "); }

        format_to(inserter, "jank::runtime::make_box<jank::runtime::obj::list>(");
        bool need_packed_comma{};
        for(size_t i{ expr.arg_exprs.size() - expr.required_packed_args.unwrap() }; i < expr.arg_exprs.size(); ++i)
        {
          if(need_packed_comma)
          { format_to(inserter, ", "); }
          gen(expr.arg_exprs[i], false);
          need_packed_comma = true;
        }
        format_to(inserter, ")");
      }

      format_to(inserter, ")");
    }

    format_to(inserter, "{}", (is_statement ? ";" : ""));
  }

  void processor::gen(analyze::expr::primitive_literal<analyze::expression> const &expr, bool const is_statement)
  {
    auto inserter(std::back_inserter(body_buffer));
    auto const &constant(expr.frame->find_lifted_constant(expr.data).unwrap().get());
    format_to
    (
      inserter,
      "{}{}",
      runtime::munge(constant.native_name.name),
      (is_statement ? ";" : "")
    );
  }

  void processor::gen
  (analyze::expr::vector<analyze::expression> const &expr, bool const is_statement)
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
    format_to(inserter, "jank::runtime::make_box<jank::runtime::obj::map>(std::in_place ");
    for(auto const &data_expr : expr.data_exprs)
    {
      format_to(inserter, ", ");
      gen(data_expr.first, false);
      format_to(inserter, ", ");
      gen(data_expr.second, false);
    }
    format_to(inserter, "){}", (is_statement ? ";" : ""));
  }

  void processor::gen(analyze::expr::local_reference const &expr, bool const is_statement)
  {
    auto inserter(std::back_inserter(body_buffer));
    format_to(inserter, "{}{}", runtime::munge(expr.name->name), (is_statement ? ";" : ""));
  }

  void processor::gen
  (analyze::expr::function<analyze::expression> const &expr, bool const is_statement)
  {
    /* Since each codegen proc handles one callable struct, we create a new one for this fn. */
    processor prc{ rt_ctx, an_ctx, expr };

    auto header_inserter(std::back_inserter(header_buffer));
    auto body_inserter(std::back_inserter(body_buffer));
    format_to(header_inserter, "{}", prc.declaration_str());
    format_to(body_inserter, "{}{}", prc.expression_str(false), (is_statement ? ";" : ""));
  }

  void processor::gen
  (analyze::expr::native_raw<analyze::expression> const &expr, bool const is_statement)
  {
    auto inserter(std::back_inserter(body_buffer));
    for(auto const &chunk : expr.chunks)
    {
      auto const * const code(boost::get<runtime::detail::string_type>(&chunk));
      if(code != nullptr)
      { format_to(inserter, "{}", *code->data); }
      else
      { gen(boost::get<analyze::expression>(chunk), false); }
    }
    format_to(inserter, "{}", (is_statement ? ";" : ""));
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
      runtime::munge(struct_name.name)
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

    bool needs_member_init{};

    for(auto const &arity : root_expression.arities)
    {
      needs_member_init |= !arity.frame->lifted_vars.empty()
                           || !arity.frame->lifted_constants.empty()
                           || !arity.frame->captures.empty();
      for(auto const &v : arity.frame->lifted_vars)
      {

        format_to
        (
          inserter,
          "jank::runtime::var_ptr const {0};", runtime::munge(v.second.native_name.name)
        );
      }

      for(auto const &v : arity.frame->lifted_constants)
      {
        format_to
        (
          inserter,
          "jank::runtime::object_ptr const {0};", runtime::munge(v.second.native_name.name)
        );
      }

      for(auto const &v : arity.frame->captures)
      {
        format_to
        (
          inserter,
          "jank::runtime::object_ptr const {0};", runtime::munge(v.first->name)
        );
      }
    }

    /* TODO: Prevent name collision with rt_ctx. */
    format_to
    (
      inserter,
      "{0}(jank::runtime::context &rt_ctx", runtime::munge(struct_name.name)
    );

    for(auto const &arity : root_expression.arities)
    {
      for(auto const &v : arity.frame->captures)
      {
        format_to
        (
          inserter,
          ", jank::runtime::object_ptr const &{0}", runtime::munge(v.first->name)
        );
      }
    }

    format_to(inserter, ")");

    if(needs_member_init)
    { format_to(inserter, " : "); }

    bool need_member_init_comma{};
    for(auto const &arity : root_expression.arities)
    {
      for(auto const &v : arity.frame->lifted_vars)
      {
        format_to
        (
          inserter,
          R"({0}{1}{{ rt_ctx.intern_var("{2}", "{3}").expect_ok() }})",
          (need_member_init_comma ? "," : ""),
          runtime::munge(v.second.native_name.name),
          v.second.var_name->ns,
          v.second.var_name->name
        );
        need_member_init_comma = true;
      }

      for(auto const &v : arity.frame->lifted_constants)
      {
        format_to
        (
          inserter,
          "{0}{1}{{",
          (need_member_init_comma ? "," : ""),
          runtime::munge(v.second.native_name.name)
        );
        detail::gen_constant(v.second.data, header_buffer);
        format_to(inserter, "}}");
        need_member_init_comma = true;
      }

      for(auto const &v : arity.frame->captures)
      {
        format_to
        (
          inserter,
          "{0}{1}{{ {1} }}",
          (need_member_init_comma ? "," : ""),
          runtime::munge(v.first->name)
        );
        need_member_init_comma = true;
      }
    }

    format_to(inserter, "{{ }}");
  }

  void processor::build_body()
  {
    auto inserter(std::back_inserter(body_buffer));

    option<size_t> required_arity;
    for(auto const &arity : root_expression.arities)
    {
      if(arity.is_variadic)
      { required_arity = arity.params.size() - 1; }

      format_to(inserter, "jank::runtime::object_ptr call(");
      bool param_comma{};
      for(auto const &param : arity.params)
      {
        format_to
        (
          inserter,
          "{} jank::runtime::object_ptr const &{}",
          (param_comma ? ", " : ""),
          runtime::munge(param->name)
        );
        param_comma = true;
      }
      format_to
      (
        inserter,
        R"(
          ) const override {{
          using namespace jank;
          using namespace jank::runtime;
        )"
      );

      for(auto it(arity.body.body.begin()); it != arity.body.body.end();)
      {
        auto const &form(*it);
        if
        (
          ++it == arity.body.body.end()
          /* Ending a fn in a native/raw expression assumes it will be returning something
           * explicitly. */
          && boost::get<analyze::expr::native_raw<analyze::expression>>(&form.data) == nullptr
        )
        { format_to(inserter, "return "); }
        gen(form, true);
      }

      format_to(inserter, "}}");
    }

    if(required_arity.is_some())
    {
      format_to
      (
        inserter,
        "jank::option<size_t> get_required_arity() const override{{ return {}; }}",
        required_arity.unwrap()
      );
    }
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
          runtime::munge(struct_name.name),
          runtime::munge(tmp_name.name),
          fmt::ptr(&rt_ctx)
        );

        for(auto const &arity : root_expression.arities)
        {
          for(auto const &v : arity.frame->captures)
          { format_to(inserter, ", {0}", runtime::munge(v.first->name)); }
        }

        format_to(inserter, "}};");

        format_to
        (
          inserter,
          "{}.call();",
          runtime::munge(tmp_name.name)
        );
      }
      else
      {
        format_to
        (
          inserter,
          "jank::runtime::make_box<{0}>(std::ref(*reinterpret_cast<jank::runtime::context*>({1}))",
          runtime::munge(struct_name.name),
          fmt::ptr(&rt_ctx)
        );

        for(auto const &arity : root_expression.arities)
        {
          for(auto const &v : arity.frame->captures)
          { format_to(inserter, ", {0}", runtime::munge(v.first->name)); }
        }

        format_to(inserter, ")");
      }

      generated_expression = true;
    }
    return { expression_buffer.data(), expression_buffer.size() };
  }
}
