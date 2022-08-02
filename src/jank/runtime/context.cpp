#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/string.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/analyze/processor.hpp>

namespace jank::runtime
{
  context::context()
  {
    auto &t_state(get_thread_state());
    auto const core(intern_ns(obj::symbol::create("clojure.core")));
    auto const locked_core_vars(core->vars.wlock());
    auto const ns_sym(obj::symbol::create("*ns*"));
    auto const ns_res(locked_core_vars->insert({ns_sym, var::create(core, ns_sym, core)}));
    t_state.current_ns = ns_res.first->second;

    auto const in_ns_sym(obj::symbol::create("in-ns"));
    std::function<object_ptr (object_ptr const&)> in_ns_fn
    (
      [this](object_ptr const &sym)
      {
        obj::symbol const * const s(sym->as_symbol());
        if(!s)
        {
          /* TODO: throw?. */
          return JANK_NIL;
        }
        auto typed_sym(boost::static_pointer_cast<obj::symbol>(sym));
        auto new_ns(intern_ns(typed_sym));
        get_thread_state().current_ns->set_root(new_ns);
        return JANK_NIL;
      }
    );
    auto const in_ns_res(locked_core_vars->insert({in_ns_sym, var::create(core, in_ns_sym, obj::function::create(in_ns_fn))}));
    t_state.in_ns = in_ns_res.first->second;

    /* TODO: Remove this once it can be defined in jank. */
    auto const println_sym(obj::symbol::create("println"));
    std::function<object_ptr (object_ptr const&)> println_fn
    (
      [](object_ptr const &o)
      {
        std::cout << o->to_string() << std::endl;
        return JANK_NIL;
      }
    );
    locked_core_vars->insert({println_sym, var::create(core, println_sym, obj::function::create(println_fn))});

    /* TODO: Remove this once it can be defined in jank. */
    auto const plus_sym(obj::symbol::create("+"));
    locked_core_vars->insert({plus_sym, var::create(core, plus_sym, obj::function::create(&obj::_gen_plus_))});
  }

  option<var_ptr> context::find_var(obj::symbol_ptr const &sym)
  {
    if(!sym->ns.empty())
    {
      ns_ptr ns;
      {
        decltype(namespaces)::DataType::iterator found;
        auto const locked_namespaces(namespaces.rlock());
        found = locked_namespaces.asNonConstUnsafe().find(runtime::obj::symbol::create(sym->ns));
        if(found == locked_namespaces->end())
        { return none; }
        ns = found->second;
      }

      {
        decltype(ns->vars)::DataType::iterator found;
        auto const locked_vars(ns->vars.rlock());
        found = locked_vars.asNonConstUnsafe().find(runtime::obj::symbol::create(sym->name));
        if(found == locked_vars->end())
        { return none; }

        return { found->second };
      }
    }
    else
    {
      auto const locked_vars(get_thread_state().current_ns->get_root()->as_ns()->vars.rlock());
      auto const found(locked_vars->find(sym));
      if(found == locked_vars->end())
      { return none; }

      return { found->second };
    }
  }
  option<object_ptr> context::find_local(obj::symbol_ptr const &)
  {
    return {};
  }
  option<object_ptr> context::find_val(obj::symbol_ptr const &sym)
  {
    auto const var(find_var(sym));
    if(var.is_some())
    { return var.unwrap()->get_root(); }

    /* TODO: Return val. */
    return find_local(sym);
  }

  object_ptr context::eval_string(std::string_view const &s)
  {
    read::lex::processor l_prc{ s };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    analyze::context anal_ctx;
    analyze::processor anal_prc{ *this };
    evaluate::context eval_ctx{  *this };

    runtime::object_ptr result;
    for(auto const &form : p_prc)
    { result = eval_ctx.eval(anal_prc.analyze(form.expect_ok(), anal_ctx)); }
    return result;
  }

  void context::dump() const
  {
    std::cout << "context dump" << std::endl;
    for(auto p : *namespaces.rlock())
    {
      std::cout << "  " << p.second->name->to_string() << std::endl;
      for(auto vp : *p.second->vars.rlock())
      {
        std::cout << "    " << vp.second->to_string() << " = " << vp.second->get_root()->to_string() << std::endl;
      }
    }
  }

  ns_ptr context::intern_ns(obj::symbol_ptr const &sym)
  {
    auto locked_namespaces(namespaces.ulock());
    auto const found(locked_namespaces->find(sym));
    if(found != locked_namespaces->end())
    { return found->second; }

    auto const write_locked_namespaces(locked_namespaces.moveFromUpgradeToWrite());
    auto const result(write_locked_namespaces->emplace(sym, make_box<ns>(sym, *this)));
    return result.first->second;
  }

  context::thread_state::thread_state(context &ctx)
    : rt_ctx{ ctx }, eval_ctx{ ctx }
  { }

  context::thread_state& context::get_thread_state()
  { return get_thread_state(none); }
  context::thread_state& context::get_thread_state(option<thread_state> init)
  {
    auto const this_id(std::this_thread::get_id());
    decltype(thread_states)::DataType::iterator found;

    /* Assume it's there and use a read lock. */
    {
      auto const locked_thread_states(thread_states.rlock());
      found = locked_thread_states.asNonConstUnsafe().find(this_id);
      if(found != locked_thread_states->end())
      { return found->second; }
    }

    /* If it's not there, use a write lock and put it there (but check again first). */
    {
      auto const locked_thread_states(thread_states.wlock());
      found = locked_thread_states->find(this_id);
      if(found != locked_thread_states->end())
      { return found->second; }
      else if(init.is_some())
      { found = locked_thread_states->emplace(this_id, std::move(init.unwrap())).first; }
      else
      { found = locked_thread_states->emplace(this_id, thread_state{ *this }).first; }
      return found->second;
    }
  }
}
