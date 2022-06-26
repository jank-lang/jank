#include <iostream>

#include <jank/runtime/obj/vector.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/analyze/expr/primitive_literal.hpp>

namespace jank::analyze
{
  expression processor::analyze_def(runtime::obj::list_ptr const &l, frame<expression> &)
  {
    auto const length(l->count());
    if(length != 3)
    {
      /* TODO: Error handling. */
      throw "invalid def";
    }

    auto const sym_obj(l->data.rest().first().unwrap());
    auto const sym(sym_obj->as_symbol());
    if(sym == nullptr)
    {
      /* TODO: Error handling. */
      throw "invalid def";
    }
    else if(!sym->ns.empty())
    {
      /* TODO: Error handling. */
      throw "invalid def";
    }

    auto const value(l->data.rest().rest().first().unwrap());
    if(value == nullptr)
    {
      /* TODO: Error handling. */
      throw "invalid def";
    }

    return
    {
      expr::def<expression>
      {
        boost::static_pointer_cast<runtime::obj::symbol>(sym_obj),
        value
      }
    };
  }
  expression processor::analyze_symbol(runtime::obj::symbol_ptr const &sym, frame<expression> &current_frame)
  {
    auto const found_local(current_frame.find(sym));
    if(found_local.is_some())
    { return { expr::local_reference<expression>{ sym, found_local.unwrap() } }; }

    auto const var(root_frame.runtime_ctx.find_var(sym));
    if(var.is_none())
    { throw "unbound symbol"; }
    return { expr::var_deref<expression>{ var.unwrap() } };
  }
  expression processor::analyze_fn(runtime::obj::list_ptr const &list, frame<expression> &current_frame)
  {
    auto const length(list->count());
    if(length < 2)
    { throw "fn missing parameter vector"; }

    auto const params_obj(list->data.rest().first().unwrap());
    auto const params(params_obj->as_vector());
    if(params == nullptr)
    { throw "invalid fn param vector"; }

    if(params->data.size() > 10)
    { throw "invalid parameter count; must be <= 10; use & args to capture the rest"; }

    frame<expression> local_frame{ "anon fn", current_frame.runtime_ctx, current_frame };
    std::vector<runtime::obj::symbol_ptr> param_symbols;
    param_symbols.reserve(params->data.size());

    for(auto const &p : params->data)
    {
      auto const sym(p->as_symbol());
      if(sym == nullptr)
      { throw "invalid parameter; must be a symbol"; }
      else if(!sym->ns.empty())
      { throw "invalid parameter; must be unqualified"; }

      auto const sym_ptr(boost::static_pointer_cast<runtime::obj::symbol>(p));
      local_frame.locals.emplace(sym_ptr, local_binding<expression>{ sym_ptr, none });
      param_symbols.emplace_back(sym_ptr);
    }

    std::list<expression> body;
    for(auto const &item : list->data.rest().rest())
    { body.push_back(analyze(item, local_frame)); }

    return { expr::function<expression>{ std::move(param_symbols), std::move(body), std::move(local_frame) } };
  }
  expression processor::analyze_let(runtime::obj::list_ptr const &, frame<expression> &)
  { return {}; }
  expression processor::analyze_if(runtime::obj::list_ptr const &, frame<expression> &)
  { return {}; }
  expression processor::analyze_quote(runtime::obj::list_ptr const &o, frame<expression> &current_frame)
  {
    if(o->count() != 2)
    { throw "invalid quote: expects one argument"; }

    return analyze_primitive_literal(o->data.rest().first().unwrap(), current_frame);
  }
  expression processor::analyze_primitive_literal(runtime::object_ptr const &o, frame<expression> &)
  {
    /* TODO: Dedupe literals. */
    return { expr::primitive_literal<expression>{ o } };
  }

  expression processor::analyze_vector(runtime::obj::vector_ptr const &o, frame<expression> &current_frame)
  {
    std::vector<expression> exprs;
    exprs.reserve(o->count());
    for(auto d = o->seq(); d != nullptr; d = d->next())
    { exprs.emplace_back(analyze(d->first(), current_frame)); }
    return { expr::vector<expression>{ std::move(exprs) } };
  }

  expression processor::analyze_call(runtime::obj::list_ptr const &o, frame<expression> &current_frame)
  {
    /* An empty list evaluates to a list, not a call. */
    auto const count(o->count());
    if(count == 0)
    { return analyze_primitive_literal(o, current_frame); }

    auto const o_seq(o->seq());
    auto const first(o_seq->first());
    if(first->as_symbol())
    {
      auto const sym(boost::static_pointer_cast<runtime::obj::symbol>(first));
      auto const found_special(specials.find(sym));
      if(found_special != specials.end())
      { return found_special->second(o, current_frame); }

      auto const found_sym(root_frame.runtime_ctx.find_val(sym));
      if(found_sym.is_none())
      { throw "cannot call unbound symbol"; }
      else if(!found_sym.unwrap()->as_callable())
      { throw "value is not callable"; }

      std::vector<expression> arg_exprs;
      arg_exprs.reserve(count - 1);
      for(auto s = o_seq->next(); s != nullptr; s = s->next())
      { arg_exprs.emplace_back(analyze(s->first())); }

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
    throw "";
  }

  processor::processor(runtime::context &rt_ctx)
    : root_frame{ "root", rt_ctx, none }
  {
    using runtime::obj::symbol;
    auto const make_fn = [this](auto const fn) -> decltype(specials)::mapped_type
    {
      return [this, fn](auto const &list, auto &current_frame)
      { return (this->*fn)(list, current_frame); };
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

  expression processor::analyze(runtime::object_ptr const &o)
  { return analyze(o, root_frame); }

  expression processor::analyze(runtime::object_ptr const &o, frame<expression> &current_frame)
  {
    assert(o);

    if(o->as_list())
    { return analyze_call(boost::static_pointer_cast<runtime::obj::list>(o), current_frame); }
    else if(o->as_vector())
    { return analyze_vector(boost::static_pointer_cast<runtime::obj::vector>(o), current_frame); }
    else if(auto * const map = o->as_map())
    {
    }
    else if(auto * const set = o->as_set())
    {
    }
    else if(o->as_number())
    { return analyze_primitive_literal(o, current_frame); }
    else if(auto * const string = o->as_string())
    {
    }
    else if(o->as_symbol())
    { return analyze_symbol(boost::static_pointer_cast<runtime::obj::symbol>(o), current_frame); }
    else if(auto * const nil = o->as_nil())
    { return analyze_primitive_literal(o, current_frame); }
    else
    {
      std::cout << "unsupported analysis of " << o->to_string() << std::endl;
      assert(false);
      throw nullptr;
    }

    return {};
  }
}
