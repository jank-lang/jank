#include <iostream>

#include <jank/analyze/processor.hpp>
#include <jank/analyze/expr/literal.hpp>

namespace jank::analyze
{
  context::context(runtime::context &ctx, std::string const &label, option<std::reference_wrapper<context>> const &p)
    : debug_label{ label }, parent{ p }, runtime_ctx{ ctx }
  { }

  expression processor::analyze_def(runtime::obj::list_ptr const &l)
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

    return { expr::def<expression>{ boost::static_pointer_cast<runtime::obj::symbol>(sym_obj), value } };
  }
  expression processor::analyze_symbol(runtime::obj::symbol_ptr const &sym)
  {
    auto const var(ctx.runtime_ctx.find_var(sym));
    if(var.is_none())
    { throw "unbound symbol"; }
    return { expr::var_deref<expression>{ var.unwrap() } };
  }
  expression processor::analyze_fn(runtime::obj::list_ptr const &)
  { return {}; }
  expression processor::analyze_let(runtime::obj::list_ptr const &)
  { return {}; }
  expression processor::analyze_if(runtime::obj::list_ptr const &)
  { return {}; }
  expression processor::analyze_quote(runtime::obj::list_ptr const &o)
  {
    if(o->count() != 2)
    { throw "invalid quote: expects one argument"; }

    return analyze_literal(o->data.rest().first().unwrap());
  }
  expression processor::analyze_literal(runtime::object_ptr const &o)
  {
    /* TODO: Dedupe literals. */
    return { expr::literal<expression>{ o } };
  }

  expression processor::analyze_call(runtime::obj::list_ptr const &o)
  {
    /* An empty list evaluates to a list, not a call. */
    auto const count(o->count());
    if(count == 0)
    { return analyze_literal(o); }

    auto const o_seq(o->seq());
    auto const first(o_seq->first());
    if(first->as_symbol())
    {
      auto const sym(boost::static_pointer_cast<runtime::obj::symbol>(first));
      auto const found_special(specials.find(sym));
      if(found_special != specials.end())
      { return found_special->second(o); }

      auto const found_sym(ctx.runtime_ctx.find_val(sym));
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
  }

  processor::processor(runtime::context &rt_ctx)
    : ctx{ rt_ctx, "root", none }
  {
    using runtime::obj::symbol;
    auto const make_fn = [this](auto const fn) -> decltype(specials)::mapped_type
    {
      return [this, fn](auto const &list)
      { return (this->*fn)(list); };
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
  {
    assert(o);

    if(o->as_list())
    { return analyze_call(boost::static_pointer_cast<runtime::obj::list>(o)); }
    else if(auto * const vector = o->as_vector())
    {
    }
    else if(auto * const map = o->as_map())
    {
    }
    else if(auto * const set = o->as_set())
    {
    }
    else if(auto * const number = o->as_number())
    {
    }
    else if(auto * const string = o->as_string())
    {
    }
    else if(o->as_symbol())
    { return analyze_symbol(boost::static_pointer_cast<runtime::obj::symbol>(o)); }
    else if(auto * const nil = o->as_nil())
    {
    }
    else
    {
      std::cout << "unsupported analysis of " << o->to_string() << std::endl;
      assert(false);
    }

    return {};
  }
}
