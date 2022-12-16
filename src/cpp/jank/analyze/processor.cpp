#include <iostream>
#include <atomic>

#include <fmt/core.h>

#include <jank/runtime/obj/vector.hpp>
#include <jank/runtime/obj/map.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/analyze/expr/primitive_literal.hpp>
#include <jank/result.hpp>

namespace jank::analyze
{
  processor::processor
  (
    runtime::context &rt_ctx
  )
    : rt_ctx{ rt_ctx }
    , root_frame{ std::make_shared<local_frame>(local_frame::frame_type::root, rt_ctx, none) }
  {
    using runtime::obj::symbol;
    auto const make_fn = [this](auto const fn) -> decltype(specials)::mapped_type
    {
      return [this, fn](auto const &list, auto &current_frame, auto expr_type, auto fn_ctx)
      { return (this->*fn)(list, current_frame, expr_type, fn_ctx); };
    };
    specials =
    {
      { symbol::create("def"), make_fn(&processor::analyze_def) },
      { symbol::create("fn*"), make_fn(&processor::analyze_fn) },
      { symbol::create("recur"), make_fn(&processor::analyze_recur) },
      { symbol::create("do"), make_fn(&processor::analyze_do) },
      { symbol::create("let*"), make_fn(&processor::analyze_let) },
      { symbol::create("if"), make_fn(&processor::analyze_if) },
      { symbol::create("quote"), make_fn(&processor::analyze_quote) },
      { symbol::create("var"), make_fn(&processor::analyze_var) },
      { symbol::create("native/raw"), make_fn(&processor::analyze_native_raw) },
    };
  }

  processor::expression_result processor::analyze
  (
    read::parse::processor::iterator parse_current,
    read::parse::processor::iterator const &parse_end
  )
  {
    if(parse_current == parse_end)
    { return err(error{ "already retrieved result" }); }

    /* We wrap all of the expressions we get in an anonymous fn so that we can call it easily.
     * This also simplifies codegen, since we only ever codegen a single fn, even if that fn
     * represents a ns, a single REPL expression, or an actual source fn. */
    runtime::detail::vector_transient_type fn;
    fn.push_back(runtime::obj::symbol::create("fn*"));
    fn.push_back(runtime::obj::vector::create());
    for(; parse_current != parse_end; ++parse_current)
    {
      if(parse_current->is_err())
      { return err(parse_current->expect_err_move()); }
      fn.push_back(parse_current->expect_ok());
    }
    auto fn_list(runtime::obj::list::create(fn.rbegin(), fn.rend()));
    return analyze(std::move(fn_list), expression_type::expression);
  }

  processor::expression_result processor::analyze_def
  (
    runtime::obj::list_ptr const &l,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const &fn_ctx
  )
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
      auto value_result(analyze(value_opt.unwrap(), current_frame, expression_type::expression, fn_ctx));
      if(value_result.is_err())
      { return value_result; }
      value_expr = some(value_result.expect_ok());
    }

    auto const qualified_sym(current_frame->lift_var(boost::static_pointer_cast<runtime::obj::symbol>(sym_obj)));
    return std::make_shared<expression>
    (
      expr::def<expression>
      {
        { expr_type },
        qualified_sym,
        value_expr,
        current_frame
      }
    );
  }

  processor::expression_result processor::analyze_symbol
  (
    runtime::obj::symbol_ptr const &sym,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const&
  )
  {
    /* TODO: Assert it doesn't start with __. */
    auto const found_local(current_frame->find_capture(sym));
    if(found_local.is_some())
    {
      local_frame::register_captures(found_local.unwrap());
      return std::make_shared<expression>
      (expr::local_reference{ { expr_type }, sym, found_local.unwrap().binding });
    }

    auto const qualified_sym(rt_ctx.qualify_symbol(sym));
    auto const var(rt_ctx.find_var(qualified_sym));
    if(var.is_none())
    { return err(error{ "unbound symbol: " + sym->to_string().data }); }

    current_frame->lift_var(qualified_sym);
    return std::make_shared<expression>
    (expr::var_deref<expression>{ { expr_type }, qualified_sym, current_frame });
  }

  result<expr::function_arity<expression>, error> processor::analyze_fn_arity
  (
    runtime::obj::list_ptr const &list,
    local_frame_ptr &current_frame
  )
  {
    auto const params_obj(list->data.first().unwrap());
    auto const * const params(params_obj->as_vector());
    if(params == nullptr)
    { return err(error{ "invalid fn parameter vector" }); }

    local_frame_ptr frame
    { std::make_shared<local_frame>(local_frame::frame_type::fn, current_frame->rt_ctx, current_frame) };
    std::vector<runtime::obj::symbol_ptr> param_symbols;
    param_symbols.reserve(params->data.size());

    bool is_variadic{};
    for(auto it(params->data.begin()); it != params->data.end(); ++it)
    {
      auto const &p(*it);
      auto const * const sym(p->as_symbol());
      if(sym == nullptr)
      { return err(error{ "invalid parameter; must be a symbol" }); }
      else if(!sym->ns.empty())
      { return err(error{ "invalid parameter; must be unqualified" }); }
      else if(sym->name == "&")
      {
        if(is_variadic)
        { return err(error{ "invalid function; parameters contain mutliple &" }); }
        else if(it + 1 == params->data.end())
        { return err(error{ "invalid function; missing symbol after &" }); }

        is_variadic = true;
        continue;
      }

      auto const sym_ptr(boost::static_pointer_cast<runtime::obj::symbol>(p));
      frame->locals.emplace(sym_ptr, local_binding{ sym_ptr, none });
      param_symbols.emplace_back(sym_ptr);
    }

    /* We do this after building the symbols vector, since the & symbol isn't a param
     * and would cause an off-by-one error. */
    if(param_symbols.size() > runtime::max_params)
    {
      return err
      (
        error
        {
          fmt::format
          (
            "invalid parameter count; must be <= {}; use & args to capture the rest",
            runtime::max_params
          )
        }
      );
    }

    auto fn_ctx(std::make_shared<expr::function_context>());
    fn_ctx->is_variadic = is_variadic;
    fn_ctx->param_count = param_symbols.size();
    expr::do_<expression> body_do;
    size_t const form_count{ list->count() - 1 };
    size_t i{};
    for(auto const &item : list->data.rest())
    {
      auto const expr_type
      ((++i == form_count) ? expression_type::return_statement : expression_type::statement);
      auto form(analyze(item, frame, expr_type, fn_ctx));
      if(form.is_err())
      { return form.expect_err_move(); }
      body_do.body.emplace_back(form.expect_ok());
    }

    return
    {
      expr::function_arity<expression>
      {
        std::move(param_symbols),
        std::move(body_do),
        std::move(frame),
        std::move(fn_ctx)
      }
    };
  }

  processor::expression_result processor::analyze_fn
  (
    runtime::obj::list_ptr const &list,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const&
  )
  {
    auto const length(list->count());
    if(length < 2)
    { return err(error{ "fn missing forms" }); }

    std::vector<expr::function_arity<expression>> arities;

    auto const first_elem(list->data.rest().first().unwrap());
    if(first_elem->as_vector() != nullptr)
    {
      auto result
      (
        analyze_fn_arity
        (runtime::make_box<runtime::obj::list>(list->data.rest()), current_frame)
      );
      if(result.is_err())
      { return result.expect_err_move(); }
      arities.emplace_back(result.expect_ok_move());
    }
    else if(first_elem->as_list() != nullptr)
    {
      for(auto it(list->data.rest()); it.size() > 0; it = it.rest())
      {
        auto arity_list(it.first().unwrap());
        if(arity_list->as_list() == nullptr)
        { return err(error{ "invalid fn: expected arity list" }); }

        auto result
        (
          analyze_fn_arity
          (boost::static_pointer_cast<runtime::obj::list>(arity_list), current_frame)
        );
        if(result.is_err())
        { return result.expect_err_move(); }
        arities.emplace_back(result.expect_ok_move());
      }
    }
    else
    { return err(error{ "invalid fn syntax" }); }

    /* There can only be one variadic arity. Clojure requires this. */
    size_t found_variadic{};
    size_t variadic_arity{};
    for(auto const &arity : arities)
    {
      found_variadic += static_cast<int>(arity.fn_ctx->is_variadic);
      variadic_arity = arity.params.size();
    }
    if(found_variadic > 1)
    { return err(error{ "invalid fn: has more than one variadic arity" }); }

    /* The variadic arity, if present, must have at least as many fixed params as the
     * highest non-variadic arity. Clojure requires this. */
    if(found_variadic > 0)
    {
      for(auto const &arity : arities)
      {
        if(!arity.fn_ctx->is_variadic && arity.params.size() >= variadic_arity)
        { return err(error{ "invalid fn: fixed arity has >= params than variadic arity" }); }
      }
    }

    /* Assert that arities are unique. Lazy implementation, but N is small anyway. */
    for(auto base(arities.begin()); base != arities.end(); ++base)
    {
      if(base + 1 == arities.end())
      { break; }

      for(auto other(base + 1); other != arities.end(); ++other)
      {
        if
        (
          base->params.size() == other->params.size()
          && base->fn_ctx->is_variadic == other->fn_ctx->is_variadic
        )
        { return err(error{ "invalid fn: duplicate arity definition" }); }
      }
    }

    return std::make_shared<expression>(expr::function<expression> { { expr_type }, none, std::move(arities) });
  }

  processor::expression_result processor::analyze_recur
  (
    runtime::obj::list_ptr const &list,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const &fn_ctx
  )
  {
    if(fn_ctx.is_none())
    { return err(error{ "unable to use recur outside of a function or loop" }); }
    else if(expr_type != expression_type::return_statement)
    { return err(error{ "recur used outside of tail position" }); }

    /* Minus one to remove recur symbol. */
    auto const arg_count(list->count() - 1);
    if(fn_ctx.unwrap()->param_count != arg_count)
    {
      return err
      (
        error
        {
          fmt::format
          (
            "invalid number of args passed to recur; expected {}, found {}",
            fn_ctx.unwrap()->param_count,
            arg_count
          )
        }
      );
    }


    std::vector<expression_ptr> arg_exprs;
    arg_exprs.reserve(arg_count);
    for(auto const &form : list->data.rest())
    {
      auto arg_expr(analyze(form, current_frame, expression_type::expression, fn_ctx));
      if(arg_expr.is_err())
      { return arg_expr; }
      arg_exprs.emplace_back(arg_expr.expect_ok());
    }

    fn_ctx.unwrap()->is_tail_recursive = true;

    return std::make_shared<expression>
    (
      expr::recur<expression>
      {
        { expr_type },
        runtime::obj::list::create(list->data.rest()),
        arg_exprs
      }
    );
  }

  processor::expression_result processor::analyze_do
  (
    runtime::obj::list_ptr const &list,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const &fn_ctx
  )
  {
    expr::do_<expression> ret;
    ret.expr_type = expr_type;
    size_t const form_count{ list->count() - 1 };
    size_t i{};
    for(auto const &item : list->data.rest())
    {
      auto const form_type
      ((++i == form_count) ? expr_type : expression_type::statement);
      auto form(analyze(item, current_frame, form_type, fn_ctx));
      if(form.is_err())
      { return form.expect_err_move(); }
      ret.body.emplace_back(form.expect_ok());
    }

    return std::make_shared<expression>(std::move(ret));
  }

  processor::expression_result processor::analyze_let
  (
    runtime::obj::list_ptr const &o,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const &fn_ctx
  )
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
      { expr_type },
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
      auto res(analyze(val, ret.frame, expression_type::expression, fn_ctx));
      if(res.is_err())
      { return res.expect_err_move(); }
      auto it(ret.pairs.emplace_back(sym_ptr, res.expect_ok_move()));
      ret.frame->locals.emplace(sym_ptr, local_binding{ sym_ptr, some(it.second) });
    }

    size_t const form_count{ o->count() - 2 };
    size_t i{};
    for(auto const &item : o->data.rest().rest())
    {
      auto const form_type
      ((++i == form_count) ? expr_type : expression_type::statement);
      auto res(analyze(item, ret.frame, form_type, fn_ctx));
      if(res.is_err())
      { return res.expect_err_move(); }
      ret.body.body.emplace_back(res.expect_ok_move());
    }

    return std::make_shared<expression>(std::move(ret));
  }

  processor::expression_result processor::analyze_if
  (
    runtime::obj::list_ptr const &o,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const &fn_ctx
  )
  {
    auto const form_count(o->count());
    if(form_count < 3)
    { return err(error{ "invalid if: expects at least two forms" }); }
    else if(form_count > 4)
    { return err(error{ "invalid if: expects at most three forms" }); }

    auto const condition(o->data.rest().first().unwrap());
    auto condition_expr(analyze(condition, current_frame, expression_type::expression, fn_ctx));
    if(condition_expr.is_err())
    { return condition_expr.expect_err_move(); }

    auto const then(o->data.rest().rest().first().unwrap());
    auto then_expr(analyze(then, current_frame, expr_type, fn_ctx));
    if(then_expr.is_err())
    { return then_expr.expect_err_move(); }

    option<expression_ptr> else_expr_opt;
    if(form_count == 4)
    {
      auto const else_(o->data.rest().rest().rest().first().unwrap());
      auto else_expr(analyze(else_, current_frame, expr_type, fn_ctx));
      if(else_expr.is_err())
      { return else_expr.expect_err_move(); }
      else_expr_opt = else_expr.expect_ok();
    }

    return std::make_shared<expression>
    (
      expr::if_<expression>
      {
        { expr_type },
        condition_expr.expect_ok(),
        then_expr.expect_ok(),
        else_expr_opt
      }
    );
  }

  processor::expression_result processor::analyze_quote
  (
    runtime::obj::list_ptr const &o,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const &fn_ctx
  )
  {
    if(o->count() != 2)
    { return err(error{ "invalid quote: expects one argument" }); }

    return analyze_primitive_literal(o->data.rest().first().unwrap(), current_frame, expr_type, fn_ctx);
  }

  processor::expression_result processor::analyze_var
  (
    runtime::obj::list_ptr const &o,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const&
  )
  {
    if(o->count() != 2)
    { return err(error{ "invalid var reference: expects one argument" }); }

    auto const &arg(o->data.rest().first().unwrap());
    auto const * const arg_sym(arg->as_symbol());
    if(arg_sym == nullptr)
    { return err(error{ "invalid var reference: expects a symbol" }); }

    auto const qualified_sym(rt_ctx.qualify_symbol(boost::static_pointer_cast<runtime::obj::symbol>(arg)));
    auto const found_var(rt_ctx.find_var(qualified_sym));
    if(found_var.is_none())
    { return err(error{ "invalid var reference: var not found" }); }

    return std::make_shared<expression>(expr::var_ref<expression>{ { expr_type }, qualified_sym, current_frame });
  }

  processor::expression_result processor::analyze_native_raw
  (
    runtime::obj::list_ptr const &o,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const &fn_ctx
  )
  {
    if(o->count() != 2)
    { return err(error{ "invalid native/raw: expects one argument" }); }

    auto const &code(o->data.rest().first().unwrap());
    auto const * const code_str(code->as_string());
    if(code_str == nullptr)
    { return err(error{ "invalid native/raw: expects string of C++ code" }); }
    if(code_str->data.empty())
    { return std::make_shared<expression>(expr::native_raw<expression>{ { expr_type }, {}, current_frame }); }

    /* native/raw expressions are broken up into chunks of either literal C++ code or
     * interpolated jank code, the latter needing to also be analyzed. */
    decltype(expr::native_raw<expression>::chunks) chunks;
    /* TODO: Just use } for end and rely on token parsing info for when that is.
     * This requires storing line/col start/end meta in each object. */
    constexpr std::string_view interp_start{ "#{" }, interp_end{ "}#" };
    for(size_t it{}; it != std::string::npos; )
    {
      auto const next_start(code_str->data.data.find(interp_start, it));
      if(next_start == std::string::npos)
      {
        /* This is the final chunk. */
        chunks.emplace_back(std::string_view{ code_str->data.data.data() + it });
        break;
      }
      auto const next_end(code_str->data.data.find(interp_end, next_start));
      if(next_end == std::string::npos)
      { return err(error{ "no matching }$ found for native/raw interpolation" }); }

      read::lex::processor l_prc
      {
        {
          code_str->data.data.data() + next_start + interp_start.size(),
          next_end - next_start - interp_end.size()
        }
      };
      read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
      auto parsed_it(p_prc.begin());
      if(parsed_it->is_err())
      { return parsed_it->expect_err_move(); }
      auto result(analyze(parsed_it->expect_ok(), current_frame, expression_type::expression, fn_ctx));
      if(result.is_err())
      { return result.expect_err_move(); }

      if(next_start - it > 0)
      { chunks.emplace_back(std::string_view{ code_str->data.data.data() + it, next_start - it }); }
      chunks.emplace_back(result.expect_ok());
      it = next_end + interp_end.size();

      if(++parsed_it != p_prc.end())
      { return err(error{ "invalid native/raw: only one expression per interpolation" }); }
    }

    return std::make_shared<expression>
    (expr::native_raw<expression>{ { expr_type }, std::move(chunks), current_frame });
  }

  processor::expression_result processor::analyze_primitive_literal
  (
    runtime::object_ptr const &o,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const&
  )
  {
    current_frame->lift_constant(o);
    return std::make_shared<expression>
    (expr::primitive_literal<expression>{ { expr_type }, o, current_frame });
  }

  /* TODO: Test for this. */
  processor::expression_result processor::analyze_vector
  (
    runtime::obj::vector_ptr const &o,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const &fn_ctx
  )
  {
    /* TODO: Detect literal and act accordingly. */
    std::vector<expression_ptr> exprs;
    exprs.reserve(o->count());
    for(auto d = o->seq(); d != nullptr; d = d->next())
    {
      auto res(analyze(d->first(), current_frame, expression_type::expression, fn_ctx));
      if(res.is_err())
      { return res.expect_err_move(); }
      exprs.emplace_back(res.expect_ok_move());
    }
    return std::make_shared<expression>(expr::vector<expression>{ { expr_type }, std::move(exprs) });
  }

  processor::expression_result processor::analyze_map
  (
    runtime::obj::map_ptr const &o,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const &fn_ctx
  )
  {
    /* TODO: Detect literal and act accordingly. */
    std::vector<std::pair<expression_ptr, expression_ptr>> exprs;
    exprs.reserve(o->data.size());
    for(auto const &kv : o->data)
    {
      auto k_expr(analyze(kv.first, current_frame, expression_type::expression, fn_ctx));
      if(k_expr.is_err())
      { return k_expr.expect_err_move(); }
      auto v_expr(analyze(kv.second, current_frame, expression_type::expression, fn_ctx));
      if(v_expr.is_err())
      { return v_expr.expect_err_move(); }
      exprs.emplace_back(k_expr.expect_ok_move(), v_expr.expect_ok_move());
    }

    /* TODO: Uniqueness check. */
    return std::make_shared<expression>(expr::map<expression>{ { expr_type }, std::move(exprs) });
  }

  processor::expression_result processor::analyze_call
  (
    runtime::obj::list_ptr const &o,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const &fn_ctx
  )
  {
    /* An empty list evaluates to a list, not a call. */
    auto const count(o->count());
    if(count == 0)
    { return analyze_primitive_literal(o, current_frame, expr_type, fn_ctx); }

    auto const first(o->data.first().unwrap());
    expression_ptr source;
    if(first->as_symbol())
    {
      auto const sym(boost::static_pointer_cast<runtime::obj::symbol>(first));
      auto const found_special(specials.find(sym));
      if(found_special != specials.end())
      { return found_special->second(o, current_frame, expr_type, fn_ctx); }

      auto sym_result(analyze_symbol(sym, current_frame, expression_type::expression, fn_ctx));
      if(sym_result.is_err())
      { return sym_result; }

      source = sym_result.expect_ok();
    }
    else
    {
      auto callable_expr(analyze(first, current_frame, expression_type::expression, fn_ctx));
      if(callable_expr.is_err())
      { return callable_expr; }
      source = callable_expr.expect_ok();
    }

    /* TODO: Verify source is callable. */

    std::vector<expression_ptr> arg_exprs;
    arg_exprs.reserve(count - 1);
    for(auto const &s : o->data.rest())
    {
      auto arg_expr(analyze(s, current_frame, expression_type::expression, fn_ctx));
      if(arg_expr.is_err())
      { return arg_expr; }
      arg_exprs.emplace_back(arg_expr.expect_ok());
    }

    return std::make_shared<expression>
    (
      expr::call<expression>
      {
        { expr_type },
        source,
        runtime::obj::list::create(o->data.rest()),
        arg_exprs
      }
    );
  }

  processor::expression_result processor::analyze
  (
    runtime::object_ptr const &o,
    expression_type const expr_type
  )
  { return analyze(o, root_frame, expr_type, none); }

  processor::expression_result processor::analyze
  (
    runtime::object_ptr const &o,
    local_frame_ptr &current_frame,
    expression_type const expr_type,
    option<expr::function_context_ptr> const& fn_ctx
  )
  {
    if(o == nullptr)
    { return err(error{ "unexpected nullptr" }); }

    if(o->as_list())
    { return analyze_call(boost::static_pointer_cast<runtime::obj::list>(o), current_frame, expr_type, fn_ctx); }
    else if(o->as_vector())
    { return analyze_vector(boost::static_pointer_cast<runtime::obj::vector>(o), current_frame, expr_type, fn_ctx); }
    else if(o->as_map())
    { return analyze_map(boost::static_pointer_cast<runtime::obj::map>(o), current_frame, expr_type, fn_ctx); }
    else if(o->as_set())
    { return err(error{ "unimplemented analysis: set" }); }
    else if(o->as_number() || o->as_boolean() || o->as_keyword() || o->as_nil() || o->as_string())
    { return analyze_primitive_literal(o, current_frame, expr_type, fn_ctx); }
    else if(o->as_symbol())
    { return analyze_symbol(boost::static_pointer_cast<runtime::obj::symbol>(o), current_frame, expr_type, fn_ctx); }
    else
    {
      std::cerr << "unsupported analysis of " << o->to_string() << std::endl;
      return err(error{ "unimplemented analysis" });
    }
  }
}
