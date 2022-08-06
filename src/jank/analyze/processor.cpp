#include <iostream>
#include <atomic>

#include <jank/runtime/obj/vector.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/analyze/expr/primitive_literal.hpp>

namespace jank::analyze
{
  context::context(runtime::context &rt_ctx)
    : rt_ctx{ rt_ctx }
  { }

  runtime::obj::symbol_ptr context::lift_var(runtime::obj::symbol_ptr const &sym)
  {
    auto const found(lifted_vars.find(sym));
    if(found != lifted_vars.end())
    { return found->first; }

    auto qualified_sym(runtime::make_box<runtime::obj::symbol>(*sym));
    if(qualified_sym->ns.empty())
    {
      auto const state(rt_ctx.get_thread_state());
      qualified_sym->ns = state.current_ns->get_root()->as_ns()->name->name;
    }
    lifted_var lv{ unique_name("var"), qualified_sym };
    lifted_vars.emplace(qualified_sym, lv);
    return qualified_sym;
  }

  option<std::reference_wrapper<lifted_var>> context::find_lifted_var(runtime::obj::symbol_ptr const &sym)
  {
    auto const found(lifted_vars.find(sym));
    if(found != lifted_vars.end())
    { return some(std::ref(found->second)); }
    return none;
  }

  void context::lift_constant(runtime::object_ptr const &constant)
  {
    auto const found(lifted_constants.find(constant));
    if(found != lifted_constants.end())
    { return; }
    lifted_constant l{ unique_name("const"), constant };
    lifted_constants.emplace(constant, l);
  }

  option<std::reference_wrapper<lifted_constant>> context::find_lifted_constant(runtime::object_ptr const &o)
  {
    auto const found(lifted_constants.find(o));
    if(found != lifted_constants.end())
    { return some(std::ref(found->second)); }
    return none;
  }

  runtime::obj::symbol context::unique_name()
  { return unique_name("gen"); }
  runtime::obj::symbol context::unique_name(std::string const &prefix)
  {
    static std::atomic_size_t index{ 1 };
    return { "", prefix + std::to_string(index++) };
  }

  processor::iterator::value_type processor::iterator::operator *() const
  { return latest; }
  processor::iterator::pointer processor::iterator::operator ->()
  { return &latest; }
  processor::iterator& processor::iterator::operator ++()
  {
    auto next(p.next(ctx));
    if(next.is_ok() && next.expect_ok().is_none())
    { is_end = true; }
    latest = std::move(next);
    return *this;
  }
  bool processor::iterator::operator !=(processor::iterator const &rhs) const
  { return !(*this == rhs); }
  bool processor::iterator::operator ==(processor::iterator const &rhs) const
  { return is_end && rhs.is_end; }

  processor::processor
  (
    runtime::context &rt_ctx,
    read::parse::processor::iterator const &b,
    read::parse::processor::iterator const &e
  )
    : rt_ctx{ rt_ctx }, root_frame{ "root", rt_ctx, none }
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

  processor::iterator processor::begin(context &ctx)
  {
    auto first(next(ctx));
    auto const has_first(first.is_err() || (first.is_ok() && first.expect_ok().is_some()));
    return { std::move(first), ctx, *this, !has_first };
  }
  processor::iterator processor::end(context &ctx)
  { return { ok(none), ctx, *this, true }; }

  processor::expression_result processor::next(context &ctx)
  {
    if(parse_current == parse_end)
    { return ok(none); }

    if(parse_current->is_err())
    { return err(parse_current->expect_err_move()); }

    auto const form(parse_current->expect_ok());
    if(form == nullptr)
    { return ok(none); }
    ++parse_current;

    return analyze(form, ctx);
  }

  processor::expression_result processor::analyze_def(runtime::obj::list_ptr const &l, frame<expression> &current_frame, context &ctx)
  {
    auto const length(l->count());
    if(length != 3)
    {
      /* TODO: Error handling. */
      return err(error{ "invalid def" });
    }

    auto const sym_obj(l->data.rest().first().unwrap());
    auto const sym(sym_obj->as_symbol());
    if(sym == nullptr)
    {
      /* TODO: Error handling. */
      return err(error{ "invalid def" });
    }
    else if(!sym->ns.empty())
    {
      /* TODO: Error handling. */
      return err(error{ "invalid def" });
    }

    auto const value(l->data.rest().rest().first().unwrap());
    if(value == nullptr)
    {
      /* TODO: Error handling. */
      return err(error{ "invalid def" });
    }

    auto const qualified_sym(ctx.lift_var(boost::static_pointer_cast<runtime::obj::symbol>(sym_obj)));

    return
    {
      expr::def<expression>
      {
        qualified_sym,
        analyze(value, current_frame, ctx).expect_ok().unwrap()
      }
    };
  }
  processor::expression_result processor::analyze_symbol(runtime::obj::symbol_ptr const &sym, frame<expression> &current_frame, context &ctx)
  {
    auto const found_local(current_frame.find(sym));
    if(found_local.is_some())
    { return { expr::local_reference<expression>{ sym, found_local.unwrap() } }; }

    auto const var(root_frame.runtime_ctx.find_var(sym));
    if(var.is_none())
    { return err(error{ "unbound symbol" }); }

    ctx.lift_var(var.unwrap()->name);
    return { expr::var_deref<expression>{ var.unwrap() } };
  }
  processor::expression_result processor::analyze_fn(runtime::obj::list_ptr const &list, frame<expression> &current_frame, context &ctx)
  {
    auto const length(list->count());
    if(length < 2)
    { return err(error{ "fn missing parameter vector" }); }

    auto const params_obj(list->data.rest().first().unwrap());
    auto const params(params_obj->as_vector());
    if(params == nullptr)
    { return err(error{ "invalid fn param vector" }); }

    if(params->data.size() > 10)
    { return err(error{ "invalid parameter count; must be <= 10; use & args to capture the rest" }); }

    frame<expression> local_frame{ "anon fn", current_frame.runtime_ctx, current_frame };
    std::vector<runtime::obj::symbol_ptr> param_symbols;
    param_symbols.reserve(params->data.size());

    for(auto const &p : params->data)
    {
      auto const sym(p->as_symbol());
      if(sym == nullptr)
      { return err(error{ "invalid parameter; must be a symbol" }); }
      else if(!sym->ns.empty())
      { return err(error{ "invalid parameter; must be unqualified" }); }

      auto const sym_ptr(boost::static_pointer_cast<runtime::obj::symbol>(p));
      local_frame.locals.emplace(sym_ptr, local_binding<expression>{ sym_ptr, none });
      param_symbols.emplace_back(sym_ptr);
    }

    expr::do_<expression> body_do;
    for(auto const &item : list->data.rest().rest())
    { body_do.body.emplace_back(analyze(item, local_frame, ctx).expect_ok().unwrap()); }

    return { expr::function<expression>{ std::move(param_symbols), std::move(body_do), std::move(local_frame) } };
  }
  processor::expression_result processor::analyze_let(runtime::obj::list_ptr const &o, frame<expression> &current_frame, context &ctx)
  {
    if(o->count() < 2)
    { return err(error{ "invalid let: expects bindings" }); }

    auto const bindings_obj(o->data.rest().first().unwrap());
    auto const bindings(bindings_obj->as_vector());
    if(bindings == nullptr)
    { return err(error{ "invalid let* bindings: must be a vector" }); }

    auto const binding_parts(bindings->data.size());
    if(binding_parts % 2 == 1)
    { return err(error{ "invalid let* bindings: must be an even number" }); }

    expr::let<expression> ret{ {}, {}, frame<expression>{ "let*", current_frame.runtime_ctx, current_frame } };
    for(size_t i{}; i < binding_parts; i += 2)
    {
      auto const &sym_obj(bindings->data[i]);
      auto const &val(bindings->data[i + 1]);

      auto const &sym(sym_obj->as_symbol());
      if(sym == nullptr || !sym->ns.empty())
      { return err(error{ "invalid let* binding: left hand must be an unqualified symbol" }); }

      auto const sym_ptr(boost::static_pointer_cast<runtime::obj::symbol>(sym_obj));
      auto it(ret.pairs.emplace_back(sym_ptr, analyze(val, ret.local_frame, ctx).expect_ok().unwrap()));
      ret.local_frame.locals.emplace(sym_ptr, local_binding<expression>{ sym_ptr, some(std::ref(it.second)) });
      /* TODO: Rename shadowed bindings? */
    }

    for(auto const &item : o->data.rest().rest())
    { ret.body.body.emplace_back(analyze(item, ret.local_frame, ctx).expect_ok().unwrap()); }

    return { std::move(ret) };
  }
  processor::expression_result processor::analyze_if(runtime::obj::list_ptr const &, frame<expression> &, context &)
  { return err(error{ "unimplemented: if" }); }
  processor::expression_result processor::analyze_quote(runtime::obj::list_ptr const &o, frame<expression> &current_frame, context &ctx)
  {
    if(o->count() != 2)
    { return err(error{ "invalid quote: expects one argument" }); }

    return analyze_primitive_literal(o->data.rest().first().unwrap(), current_frame, ctx);
  }
  processor::expression_result processor::analyze_primitive_literal(runtime::object_ptr const &o, frame<expression> &, context &ctx)
  {
    ctx.lift_constant(o);
    return { expr::primitive_literal<expression>{ o } };
  }

  processor::expression_result processor::analyze_vector(runtime::obj::vector_ptr const &o, frame<expression> &current_frame, context &ctx)
  {
    /* TODO: Detect literal and act accordingly. */
    std::vector<expression> exprs;
    exprs.reserve(o->count());
    for(auto d = o->seq(); d != nullptr; d = d->next())
    { exprs.emplace_back(analyze(d->first(), current_frame, ctx).expect_ok().unwrap()); }
    return { expr::vector<expression>{ std::move(exprs) } };
  }

  processor::expression_result processor::analyze_call(runtime::obj::list_ptr const &o, frame<expression> &current_frame, context &ctx)
  {
    /* An empty list evaluates to a list, not a call. */
    auto const count(o->count());
    if(count == 0)
    { return analyze_primitive_literal(o, current_frame, ctx); }

    auto const o_seq(o->seq());
    auto const first(o_seq->first());
    if(first->as_symbol())
    {
      auto const sym(boost::static_pointer_cast<runtime::obj::symbol>(first));
      auto const found_special(specials.find(sym));
      if(found_special != specials.end())
      { return found_special->second(o, current_frame, ctx); }

      auto const found_sym(root_frame.runtime_ctx.find_val(sym));
      if(found_sym.is_none())
      { return err(error{ "cannot call unbound symbol" }); }
      else if(!found_sym.unwrap()->as_callable())
      { return err(error{ "value is not callable" }); }

      std::vector<expression> arg_exprs;
      arg_exprs.reserve(count - 1);
      for(auto s = o_seq->next(); s != nullptr; s = s->next())
      { arg_exprs.emplace_back(analyze(s->first(), ctx).expect_ok().unwrap()); }

      return
      {
        expr::call<expression>
        {
          boost::static_pointer_cast<runtime::obj::function>(found_sym.unwrap()),
          runtime::obj::list::create(o->data.rest()),
          arg_exprs
        }
      };
    }
    else if(auto const * const callable = first->as_callable())
    {
      // TODO: Callable
    }

    /* TODO: Proper error handling. */
    assert(false);
    return err(error{ "unreachable call" });
  }

  processor::expression_result processor::analyze(runtime::object_ptr const &o, context &ctx)
  { return analyze(o, root_frame, ctx); }

  processor::expression_result processor::analyze(runtime::object_ptr const &o, frame<expression> &current_frame, context &ctx)
  {
    assert(o);

    if(o->as_list())
    { return analyze_call(boost::static_pointer_cast<runtime::obj::list>(o), current_frame, ctx); }
    else if(o->as_vector())
    { return analyze_vector(boost::static_pointer_cast<runtime::obj::vector>(o), current_frame, ctx); }
    else if(auto * const map = o->as_map())
    {
    }
    else if(auto * const set = o->as_set())
    {
    }
    else if(o->as_number())
    { return analyze_primitive_literal(o, current_frame, ctx); }
    else if(auto * const string = o->as_string())
    {
    }
    else if(o->as_symbol())
    { return analyze_symbol(boost::static_pointer_cast<runtime::obj::symbol>(o), current_frame, ctx); }
    else if(auto * const nil = o->as_nil())
    { return analyze_primitive_literal(o, current_frame, ctx); }
    else
    {
      std::cerr << "unsupported analysis of " << o->to_string() << std::endl;
      assert(false);
      throw nullptr;
    }

    std::cerr << "unimplemented analysis of " << *o << std::endl;
    throw nullptr;
  }
}
