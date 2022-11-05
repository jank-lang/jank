#include <iostream>
#include <atomic>

#include <jank/runtime/obj/vector.hpp>
#include <jank/runtime/obj/map.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/analyze/expr/primitive_literal.hpp>

namespace jank::analyze
{
  context::context(runtime::context &rt_ctx)
    : rt_ctx{ rt_ctx }
  { }

  void context::dump() const
  {
    std::cout << "analysis ctx dump begin" << std::endl;
    for(auto const &v : vars)
    { std::cout << "  var " << *v.first << std::endl; }
    std::cout << "analysis ctx dump end" << std::endl;
  }

  option<std::pair<runtime::obj::symbol_ptr, option<expression_ptr>>> context::find_var
  (runtime::obj::symbol_ptr const &sym) const
  {
    runtime::obj::symbol_ptr qualified_sym{ sym };
    if(qualified_sym->ns.empty())
    {
      auto const t_state(rt_ctx.get_thread_state());
      auto const * const current_ns(t_state.current_ns->get_root()->as_ns());
      qualified_sym = runtime::obj::symbol::create(current_ns->name->name, sym->name);
    }

    auto const found(vars.find(qualified_sym));
    if(found == vars.end())
    { return none; }
    return some(*found);
  }

  runtime::obj::symbol context::unique_name()
  { return unique_name("gen"); }
  runtime::obj::symbol context::unique_name(std::string const &prefix)
  {
    static std::atomic_size_t index{ 1 };
    return { "", prefix + std::to_string(index++) };
  }

  processor::processor
  (
    runtime::context &rt_ctx,
    read::parse::processor::iterator const &b,
    read::parse::processor::iterator const &e
  )
    : rt_ctx{ rt_ctx }
    , root_frame{ std::make_shared<local_frame>(local_frame::frame_type::root, rt_ctx, none) }
    , parse_current{ b }, parse_end{ e }
  {
    using runtime::obj::symbol;
    auto const make_fn = [this](auto const fn) -> decltype(specials)::mapped_type
    {
      return [this, fn](auto const &list, auto &current_frame, auto &ctx)
      { return (this->*fn)(list, current_frame, ctx); };
    };
    specials =
    {
      { symbol::create("def"), make_fn(&processor::analyze_def) },
      { symbol::create("fn*"), make_fn(&processor::analyze_fn) },
      { symbol::create("let*"), make_fn(&processor::analyze_let) },
      { symbol::create("if"), make_fn(&processor::analyze_if) },
      { symbol::create("quote"), make_fn(&processor::analyze_quote) },
    };
  }

  processor::expression_result processor::result(context &ctx)
  {
    if(parse_current == parse_end)
    { return ok(none); }

    /* We wrap all of the expressions we get in an anonymous fn so that we can call it easily.
     * This also simplifies codegen, since we only ever codegen a single fn, even if that fn
     * represents a ns, a single REPL expression, or an actual source fn. */
    runtime::detail::vector_transient_type fn_body;
    fn_body.push_back(runtime::obj::symbol::create("fn*"));
    fn_body.push_back(runtime::obj::vector::create());
    for(; parse_current != parse_end; ++parse_current)
    {
      if(parse_current->is_err())
      { return err(parse_current->expect_err_move()); }
      fn_body.push_back(parse_current->expect_ok());
    }
    auto fn(runtime::obj::list::create(fn_body.rbegin(), fn_body.rend()));
    return analyze(std::move(fn), ctx);
  }

  processor::expression_result processor::analyze_def
  (runtime::obj::list_ptr const &l, local_frame_ptr &current_frame, context &ctx)
  {
    auto const length(l->count());
    if(length != 2 && length != 3)
    {
      /* TODO: Error handling. */
      return err(error{ "invalid def" });
    }

    auto const sym_obj(l->data.rest().first().unwrap());
    auto const * const sym(sym_obj->as_symbol());
    if(sym == nullptr)
    {
      /* TODO: Error handling. */
      return err(error{ "invalid def: name must be a symbol" });
    }
    else if(!sym->ns.empty())
    {
      /* TODO: Error handling. */
      return err(error{ "invalid def: name must not be qualified" });
    }

    bool has_value{ true };
    auto const value_opt(l->data.rest().rest().first());
    if(value_opt.is_none())
    { has_value = false; }

    option<std::shared_ptr<expression>> value_expr;

    if(has_value)
    {
      auto value_result(analyze(value_opt.unwrap(), current_frame, ctx));
      if(value_result.is_err())
      { return value_result; }
      value_expr = some(std::make_shared<expression>(value_result.expect_ok().unwrap()));
    }

    auto const qualified_sym(current_frame->lift_var(boost::static_pointer_cast<runtime::obj::symbol>(sym_obj)));
    auto const existing_var(ctx.vars.find(qualified_sym));
    if(existing_var != ctx.vars.end())
    {
      /* TODO: If backward checking is enabled, type check new value type with old one. */
      existing_var->second = value_expr;
    }
    else
    { ctx.vars.emplace(qualified_sym, value_expr); }

    return
    {
      expr::def<expression>
      {
        qualified_sym,
        value_expr,
        current_frame
      }
    };
  }

  processor::expression_result processor::analyze_symbol
  (runtime::obj::symbol_ptr const &sym, local_frame_ptr &current_frame, context &ctx)
  {
    auto const found_local(current_frame->find_capture(sym));
    if(found_local.is_some())
    {
      local_frame::register_captures(found_local.unwrap());
      return { expr::local_reference{ sym, found_local.unwrap().binding } };
    }

    auto const var(ctx.find_var(sym));
    if(var.is_none())
    { return err(error{ "unbound symbol: " + *sym->to_string().data }); }

    current_frame->lift_var(var.unwrap().first);
    return { expr::var_deref<expression>{ var.unwrap().first, current_frame } };
  }

  processor::expression_result processor::analyze_fn
  (runtime::obj::list_ptr const &list, local_frame_ptr &current_frame, context &ctx)
  {
    auto const length(list->count());
    if(length < 2)
    { return err(error{ "fn missing parameter vector" }); }

    auto const params_obj(list->data.rest().first().unwrap());
    auto const * const params(params_obj->as_vector());
    if(params == nullptr)
    { return err(error{ "invalid fn param vector" }); }

    if(params->data.size() > 10)
    { return err(error{ "invalid parameter count; must be <= 10; use & args to capture the rest" }); }

    local_frame_ptr frame
    { std::make_shared<local_frame>(local_frame::frame_type::fn, current_frame->rt_ctx, current_frame) };
    std::vector<runtime::obj::symbol_ptr> param_symbols;
    param_symbols.reserve(params->data.size());

    for(auto const &p : params->data)
    {
      auto const * const sym(p->as_symbol());
      if(sym == nullptr)
      { return err(error{ "invalid parameter; must be a symbol" }); }
      else if(!sym->ns.empty())
      { return err(error{ "invalid parameter; must be unqualified" }); }

      auto const sym_ptr(boost::static_pointer_cast<runtime::obj::symbol>(p));
      frame->locals.emplace(sym_ptr, local_binding{ sym_ptr, none });
      param_symbols.emplace_back(sym_ptr);
    }

    expr::do_<expression> body_do;
    for(auto const &item : list->data.rest().rest())
    {
      auto form(analyze(item, frame, ctx));
      if(form.is_err())
      { return form.expect_err_move(); }
      else if(form.expect_ok().is_none())
      { return err(read::error{ "expected fn body value" }); }
      body_do.body.emplace_back(std::move(form.expect_ok_move().unwrap()));
    }

    return
    {
      expr::function<expression>
      { std::move(param_symbols), std::move(body_do), std::move(frame) }
    };
  }

  processor::expression_result processor::analyze_let
  (runtime::obj::list_ptr const &o, local_frame_ptr &current_frame, context &ctx)
  {
    if(o->count() < 2)
    { return err(error{ "invalid let: expects bindings" }); }

    auto const bindings_obj(o->data.rest().first().unwrap());
    auto const * const bindings(bindings_obj->as_vector());
    if(bindings == nullptr)
    { return err(error{ "invalid let* bindings: must be a vector" }); }

    auto const binding_parts(bindings->data.size());
    if(binding_parts % 2 == 1)
    { return err(error{ "invalid let* bindings: must be an even number" }); }

    expr::let<expression> ret
    {
      {},
      {},
      std::make_shared<local_frame>
      (local_frame::frame_type::let, current_frame->rt_ctx, current_frame)
    };
    for(size_t i{}; i < binding_parts; i += 2)
    {
      auto const &sym_obj(bindings->data[i]);
      auto const &val(bindings->data[i + 1]);

      auto const &sym(sym_obj->as_symbol());
      if(sym == nullptr || !sym->ns.empty())
      { return err(error{ "invalid let* binding: left hand must be an unqualified symbol" }); }

      auto const sym_ptr(boost::static_pointer_cast<runtime::obj::symbol>(sym_obj));
      /* TODO: Return errors. */
      auto it(ret.pairs.emplace_back(sym_ptr, analyze(val, ret.frame, ctx).expect_ok().unwrap()));
      ret.frame->locals.emplace(sym_ptr, local_binding{ sym_ptr, some(std::ref(it.second)) });
      /* TODO: Rename shadowed bindings? */
    }

    for(auto const &item : o->data.rest().rest())
    /* TODO: Return errors. */
    { ret.body.body.emplace_back(analyze(item, ret.frame, ctx).expect_ok().unwrap()); }

    return { std::move(ret) };
  }

  processor::expression_result processor::analyze_if
  (runtime::obj::list_ptr const &, local_frame_ptr &, context &)
  { return err(error{ "unimplemented: if" }); }

  processor::expression_result processor::analyze_quote
  (runtime::obj::list_ptr const &o, local_frame_ptr &current_frame, context &ctx)
  {
    if(o->count() != 2)
    { return err(error{ "invalid quote: expects one argument" }); }

    return analyze_primitive_literal(o->data.rest().first().unwrap(), current_frame, ctx);
  }

  processor::expression_result processor::analyze_primitive_literal
  (runtime::object_ptr const &o, local_frame_ptr &current_frame, context &)
  {
    current_frame->lift_constant(o);
    return { expr::primitive_literal<expression>{ o, current_frame } };
  }

  /* TODO: Test for this. */
  processor::expression_result processor::analyze_vector
  (runtime::obj::vector_ptr const &o, local_frame_ptr &current_frame, context &ctx)
  {
    /* TODO: Detect literal and act accordingly. */
    std::vector<expression> exprs;
    exprs.reserve(o->count());
    for(auto d = o->seq(); d != nullptr; d = d->next())
    /* TODO: Return errors. */
    { exprs.emplace_back(analyze(d->first(), current_frame, ctx).expect_ok().unwrap()); }
    return { expr::vector<expression>{ std::move(exprs) } };
  }

  processor::expression_result processor::analyze_map
  (runtime::obj::map_ptr const &o, local_frame_ptr &current_frame, context &ctx)
  {
    /* TODO: Detect literal and act accordingly. */
    std::vector<std::pair<expression, expression>> exprs;
    exprs.reserve(o->data.size());
    for(auto const &kv : o->data)
    /* TODO: Return errors. */
    {
      auto const k_expr(analyze(kv.first, current_frame, ctx).expect_ok().unwrap());
      auto const v_expr(analyze(kv.second, current_frame, ctx).expect_ok().unwrap());
      exprs.emplace_back(k_expr, v_expr);
    }

    /* TODO: Uniqueness check. */
    return { expr::map<expression>{ std::move(exprs) } };
  }

  processor::expression_result processor::analyze_call
  (runtime::obj::list_ptr const &o, local_frame_ptr &current_frame, context &ctx)
  {
    /* An empty list evaluates to a list, not a call. */
    auto const count(o->count());
    if(count == 0)
    { return analyze_primitive_literal(o, current_frame, ctx); }

    auto const o_seq(o->seq());
    auto const first(o_seq->first());
    expression_ptr source;
    if(first->as_symbol())
    {
      auto const sym(boost::static_pointer_cast<runtime::obj::symbol>(first));
      auto const found_special(specials.find(sym));
      if(found_special != specials.end())
      { return found_special->second(o, current_frame, ctx); }

      auto sym_result(analyze_symbol(sym, current_frame, ctx));
      if(sym_result.is_err())
      { return sym_result; }
      source = std::make_shared<expression>(sym_result.expect_ok().unwrap());
    }
    else
    {
      auto callable_expr(analyze(first, current_frame, ctx));
      if(callable_expr.is_err())
      { return callable_expr; }
      source = std::make_shared<expression>(callable_expr.expect_ok().unwrap());
    }

    std::vector<expression> arg_exprs;
    arg_exprs.reserve(count - 1);
    for(auto const &s : o->data.rest())
    {
      auto arg_expr(analyze(s, current_frame, ctx));
      if(arg_expr.is_err())
      { return arg_expr; }
      arg_exprs.emplace_back(arg_expr.expect_ok().unwrap());
    }

    return
    {
      expr::call<expression>
      {
        source,
        runtime::obj::list::create(o->data.rest()),
        arg_exprs
      }
    };
  }

  processor::expression_result processor::analyze(runtime::object_ptr const &o, context &ctx)
  { return analyze(o, root_frame, ctx); }

  processor::expression_result processor::analyze
  (runtime::object_ptr const &o, local_frame_ptr &current_frame, context &ctx)
  {
    assert(o);

    if(o->as_list())
    { return analyze_call(boost::static_pointer_cast<runtime::obj::list>(o), current_frame, ctx); }
    else if(o->as_vector())
    { return analyze_vector(boost::static_pointer_cast<runtime::obj::vector>(o), current_frame, ctx); }
    else if(o->as_map())
    { return analyze_map(boost::static_pointer_cast<runtime::obj::map>(o), current_frame, ctx); }
    else if(o->as_set())
    { return err(error{ "unimplemented analysis: set" }); }
    else if(o->as_number() || o->as_boolean() || o->as_keyword() || o->as_nil())
    { return analyze_primitive_literal(o, current_frame, ctx); }
    else if(o->as_string())
    { return err(error{ "unimplemented analysis: string" }); }
    else if(o->as_symbol())
    { return analyze_symbol(boost::static_pointer_cast<runtime::obj::symbol>(o), current_frame, ctx); }
    else
    {
      std::cerr << "unsupported analysis of " << o->to_string() << std::endl;
      return err(error{ "unimplemented analysis" });
    }
  }
}
