#include <iostream>

#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/util.hpp>
#include <jank/codegen/processor.hpp>

/* The strategy for codegen to C++ is quite simple. Codegen always happens on a
 * single fn, which generates a single C++ struct. Top-level expressions and
 * REPL expressions are all implicitly wrapped in a fn during analysis. If the
 * jank fn has a nested fn, it becomes a nested struct, since this whole
 * generation works recursively.
 *
 * Analysis lifts constants and vars, so those just become members which are
 * initialized in the ctor.
 *
 * The most interesting part is the translation of expressions into statements,
 * so that something like `(println (if foo bar spam))` can become sane C++.
 * To do this, _every_ nested expression is replaced with a temporary and turned
 * into a statement. When needed, such as with if statements, that temporary
 * is mutated from the then/else branches. In other cases, it's just set
 * directly.
 *
 * That means something like `(println (thing) (if foo bar spam))` will become
 * roughly this C++:
 *
 * ```c++
 * object_ptr const &thing_result(thing->call());
 * object_ptr if_result;
 * if(foo)
 * { if_result = bar; }
 * else
 * { if_result = spam; }
 * println->call(thing_result, if_result);
 * ```
 *
 * TODO: This could be optimized, in some cases, but let's do that when the
 * profiler tells us to.
 */

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
      { format_to(inserter, "jank::runtime::JANK_NIL"); }
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
          R"(__rt_ctx.intern_keyword("{}", "{}", {}))",
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
    analyze::expression_ptr const &expr
  )
    : rt_ctx{ rt_ctx }, an_ctx{ an_ctx },
      root_expr{ expr },
      root_fn{ boost::get<analyze::expr::function<analyze::expression>>(expr->data) },
      struct_name{ analyze::context::unique_name() }
  { }

  processor::processor
  (
    runtime::context &rt_ctx,
    analyze::context &an_ctx,
    analyze::expr::function<analyze::expression> const &expr
  )
    : rt_ctx{ rt_ctx }, an_ctx{ an_ctx },
      root_fn{ expr },
      struct_name{ analyze::context::unique_name() }
  { }

  runtime::obj::symbol processor::gen(analyze::expression_ptr const &ex, bool const is_statement)
  {
    runtime::obj::symbol ret;
    boost::apply_visitor
    (
      [this, is_statement, &ret](auto const &typed_ex)
      { ret = gen(typed_ex, is_statement); },
      ex->data
    );
    return ret;
  }

  runtime::obj::symbol processor::gen(analyze::expr::def<analyze::expression> const &expr, bool const)
  {
    auto inserter(std::back_inserter(body_buffer));
    auto const &var(expr.frame->find_lifted_var(expr.name).unwrap().get());
    auto const &munged_name(runtime::munge(var.native_name.name));
    auto ret_tmp(analyze::context::unique_name(munged_name));

    /* Forward declarations just intern the var and evaluate to it. */
    if(expr.value.is_none())
    {
      format_to
      (
        inserter,
        "var_ptr const &{}{{{}}};",
        ret_tmp.name,
        munged_name
      );
      return ret_tmp;
    }

    auto const &val_tmp(gen(expr.value.unwrap(), false));
    format_to
    (
      inserter,
      "var_ptr const &{}{{{}->set_root({})}};",
      ret_tmp.name,
      runtime::munge(var.native_name.name),
      val_tmp.name
    );
    return ret_tmp;
  }

  runtime::obj::symbol processor::gen(analyze::expr::var_deref<analyze::expression> const &expr, bool const)
  {
    auto inserter(std::back_inserter(body_buffer));
    auto const &var(expr.frame->find_lifted_var(expr.qualified_name).unwrap().get());
    auto ret_tmp(analyze::context::unique_name(var.native_name.name));
    format_to(inserter, "auto const &{}({}->get_root());", ret_tmp.name, var.native_name.name);
    return ret_tmp;
  }

  runtime::obj::symbol processor::gen(analyze::expr::call<analyze::expression> const &expr, bool const)
  {
    /* It's worth noting that there's extra scope wrapped around the generated
     * arg values. This ensures that the args are only retained for the duration
     * of the call. Otherwise, a large, long-running fn could lead to a lot of
     * memory bloat. */
    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(analyze::context::unique_name("call"));

    /* If we don't know how many packed args there are, it's because we were
     * unable to analyze this call. Needs to be a dynamic call. */
    if(expr.required_packed_args.is_none())
    {
      auto const &source_tmp(gen(expr.source, false));
      format_to(inserter, "object_ptr {}; {{", ret_tmp.name);
      std::vector<runtime::obj::symbol> arg_tmps;
      arg_tmps.reserve(expr.arg_exprs.size());
      for(auto const &arg_expr : expr.arg_exprs)
      { arg_tmps.emplace_back(gen(arg_expr, false)); }

      format_to
      (inserter, "{} = jank::runtime::dynamic_call({}", ret_tmp.name, source_tmp.name);
      for(auto const &arg_tmp : arg_tmps)
      { format_to(inserter, ", {}", arg_tmp.name); }
      format_to(inserter, "); }}");
    }
    else
    {
      auto const &source_tmp(gen(expr.source, false));
      format_to(inserter, "object_ptr {}; {{", ret_tmp.name);
      std::vector<runtime::obj::symbol> arg_tmps;
      arg_tmps.reserve(expr.arg_exprs.size());
      for(size_t i{}; i < expr.arg_exprs.size() - expr.required_packed_args.unwrap(); ++i)
      { arg_tmps.emplace_back(gen(expr.arg_exprs[i], false)); }

      if(expr.required_packed_args.unwrap() > 0)
      {
        std::vector<runtime::obj::symbol> packed_tmps;
        packed_tmps.reserve(expr.required_packed_args.unwrap());
        for
        (
          size_t i{ expr.arg_exprs.size() - expr.required_packed_args.unwrap() };
          i < expr.arg_exprs.size();
          ++i
        )
        { packed_tmps.emplace_back(gen(expr.arg_exprs[i], false)); }

        auto packed_tmp(analyze::context::unique_name("packed"));
        format_to
        (
          inserter,
          "auto const &{}(jank::runtime::make_box<jank::runtime::obj::list>(",
          packed_tmp.name
        );
        bool need_packed_comma{};
        for(auto const &packed_tmp : packed_tmps)
        {
          format_to(inserter, "{} {}", (need_packed_comma ? ", " : ""), packed_tmp.name);
          need_packed_comma = true;
        }
        format_to(inserter, "));");
        arg_tmps.emplace_back(std::move(packed_tmp));
      }

      format_to(inserter, "{} = {}->as_callable()->call(", ret_tmp.name, source_tmp.name);
      bool need_comma{};
      for(auto const &arg_tmp : arg_tmps)
      {
        format_to(inserter, "{} {}", (need_comma ? ", " : ""), arg_tmp.name);
        need_comma = true;
      }

      format_to(inserter, "); }}");
    }

    return ret_tmp;
  }

  runtime::obj::symbol processor::gen(analyze::expr::primitive_literal<analyze::expression> const &expr, bool const)
  {
    auto const &constant(expr.frame->find_lifted_constant(expr.data).unwrap().get());
    return constant.native_name;
  }

  runtime::obj::symbol processor::gen
  (analyze::expr::vector<analyze::expression> const &expr, bool const)
  {
    std::vector<runtime::obj::symbol> data_tmps;
    data_tmps.reserve(expr.data_exprs.size());
    for(auto const &data_expr : expr.data_exprs)
    { data_tmps.emplace_back(gen(data_expr, false)); }

    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(analyze::context::unique_name("vec"));
    format_to
    (inserter, "auto const &{}(jank::runtime::make_box<jank::runtime::obj::vector>(", ret_tmp.name);
    for(auto it(data_tmps.begin()); it != data_tmps.end();)
    {
      format_to(inserter, "{}", it->name);
      if(++it != data_tmps.end())
      { format_to(inserter, ", "); }
    }
    format_to(inserter, "));");
    return ret_tmp;
  }

  runtime::obj::symbol processor::gen(analyze::expr::map<analyze::expression> const &expr, bool const)
  {
    std::vector<std::pair<runtime::obj::symbol, runtime::obj::symbol>> data_tmps;
    data_tmps.reserve(expr.data_exprs.size());
    for(auto const &data_expr : expr.data_exprs)
    { data_tmps.emplace_back(gen(data_expr.first, false), gen(data_expr.second, false)); }

    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(analyze::context::unique_name("map"));
    format_to
    (
      inserter,
      "auto const &{}(jank::runtime::make_box<jank::runtime::obj::map>(std::in_place ",
      ret_tmp.name
    );
    for(auto const &data_tmp : data_tmps)
    {
      format_to(inserter, ", {}", data_tmp.first.name);
      format_to(inserter, ", {}", data_tmp.second.name);
    }
    format_to(inserter, "));");
    return ret_tmp;
  }

  runtime::obj::symbol processor::gen(analyze::expr::local_reference const &expr, bool const)
  { return { "", runtime::munge(expr.name->name) }; }

  runtime::obj::symbol processor::gen
  (analyze::expr::function<analyze::expression> const &expr, bool const)
  {
    /* Since each codegen proc handles one callable struct, we create a new one for this fn. */
    processor prc{ rt_ctx, an_ctx, expr };
    auto ret_tmp(analyze::context::unique_name("fn"));

    auto header_inserter(std::back_inserter(header_buffer));
    auto body_inserter(std::back_inserter(body_buffer));
    format_to(header_inserter, "{}", prc.declaration_str());
    format_to(body_inserter, "auto const &{}({});", ret_tmp.name, prc.expression_str(false));
    return ret_tmp;
  }

  runtime::obj::symbol processor::gen(analyze::expr::let<analyze::expression> const &expr, bool const)
  {
    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(analyze::context::unique_name("let"));
    format_to(inserter, "object_ptr {}{{ jank::runtime::JANK_NIL }}; {{", ret_tmp.name);
    for(auto const &pair : expr.pairs)
    {
      auto const &val_tmp(gen(pair.second, false));
      auto const &munged_name(runtime::munge(pair.first->name));
      format_to(inserter, "object_ptr {}{{ {} }};", munged_name, val_tmp.name);
    }

    for(auto it(expr.body.body.begin()); it != expr.body.body.end(); )
    {
      auto const &val_tmp(gen(*it, false));

      /* We ignore all values but the last. */
      if(++it == expr.body.body.end())
      { format_to(inserter, "{} = {};", ret_tmp.name, val_tmp.name); }
    }
    format_to(inserter, "}}");

    return ret_tmp;
  }

  runtime::obj::symbol processor::gen(analyze::expr::do_<analyze::expression> const &, bool const)
  {
    return { "", "JANK_NIL" };
  }

  runtime::obj::symbol processor::gen(analyze::expr::if_<analyze::expression> const &expr, bool const)
  {
    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(analyze::context::unique_name("if"));
    format_to(inserter, "object_ptr {};", ret_tmp.name);
    auto const &condition_tmp(gen(expr.condition, false));
    format_to(inserter, "if(jank::runtime::detail::truthy({})) {{", condition_tmp.name);
    auto const &then_tmp(gen(expr.then, false));
    format_to(inserter, "{} = {}; }}", ret_tmp.name, then_tmp.name);
    if(expr.else_.is_some())
    {
      format_to(inserter, "else {{");
      auto const &else_tmp(gen(expr.else_.unwrap(), false));
      format_to(inserter, "{} = {}; }}", ret_tmp.name, else_tmp.name);
    }
    return ret_tmp;
  }

  runtime::obj::symbol processor::gen
  (analyze::expr::native_raw<analyze::expression> const &expr, bool const)
  {
    auto inserter(std::back_inserter(body_buffer));
    auto ret_tmp(analyze::context::unique_name("native"));

    std::vector<runtime::obj::symbol> interpolated_chunk_tmps;
    interpolated_chunk_tmps.reserve((expr.chunks.size() / 2) + 1);
    for(auto const &chunk : expr.chunks)
    {
      auto const * const chunk_expr(boost::get<analyze::expression_ptr>(&chunk));
      if(chunk_expr == nullptr)
      { continue; }
      interpolated_chunk_tmps.emplace_back(gen(*chunk_expr, false));
    }

    format_to(inserter, "object_ptr {};", ret_tmp.name);
    format_to(inserter, "{{ object_ptr __value{{ JANK_NIL }};");
    size_t interpolated_chunk_it{};
    for(auto const &chunk : expr.chunks)
    {
      auto const * const code(boost::get<runtime::detail::string_type>(&chunk));
      if(code != nullptr)
      { format_to(inserter, "{}", code->data); }
      else
      { format_to(inserter, "{}", interpolated_chunk_tmps[interpolated_chunk_it++].name); }
    }
    format_to(inserter, ";{} = __value; }}", ret_tmp.name);
    return ret_tmp;
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

    for(auto const &arity : root_fn.arities)
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

    format_to
    (
      inserter,
      "{0}(jank::runtime::context &__rt_ctx", runtime::munge(struct_name.name)
    );

    for(auto const &arity : root_fn.arities)
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
    for(auto const &arity : root_fn.arities)
    {
      for(auto const &v : arity.frame->lifted_vars)
      {
        format_to
        (
          inserter,
          R"({0}{1}{{ __rt_ctx.intern_var("{2}", "{3}").expect_ok() }})",
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
    for(auto const &arity : root_fn.arities)
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
        auto const &val_tmp(gen(form, true));
        if(++it == arity.body.body.end())
        { format_to(inserter, "return {};", val_tmp.name); }
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

        for(auto const &arity : root_fn.arities)
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

        for(auto const &arity : root_fn.arities)
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
