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
    auto const ns_sym(obj::symbol::create("clojure.core/*ns*"));
    auto const ns_res(locked_core_vars->insert({ns_sym, var::create(core, ns_sym, core)}));
    t_state.current_ns = ns_res.first->second;

    auto const in_ns_sym(obj::symbol::create("clojure.core/in-ns"));
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
    auto const println_sym(obj::symbol::create("clojure.core/println"));
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
    auto const plus_sym(obj::symbol::create("clojure.core/+"));
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
        found = locked_namespaces.asNonConstUnsafe().find(runtime::obj::symbol::create("", sym->ns));
        if(found == locked_namespaces->end())
        { return none; }
        ns = found->second;
      }

      {
        decltype(ns->vars)::DataType::iterator found;
        auto const locked_vars(ns->vars.rlock());
        found = locked_vars.asNonConstUnsafe().find(sym);
        if(found == locked_vars->end())
        { return none; }

        return { found->second };
      }
    }
    else
    {
      auto const t_state(get_thread_state());
      auto const current_ns(t_state.current_ns->get_root()->as_ns());
      auto const locked_vars(current_ns->vars.rlock());
      auto const qualified_sym(runtime::obj::symbol::create(current_ns->name->name, sym->name));
      auto const found(locked_vars->find(qualified_sym));
      if(found == locked_vars->end())
      { return none; }

      return { found->second };
    }
  }
  option<object_ptr> context::find_local(obj::symbol_ptr const &)
  {
    return none;
  }
  option<object_ptr> context::find_val(obj::symbol_ptr const &sym)
  {
    auto const var(find_var(sym));
    if(var.is_some())
    { return some(var.unwrap()->get_root()); }

    /* TODO: Return val. */
    return find_local(sym);
  }

  object_ptr context::eval_string(std::string_view const &s)
  {
    read::lex::processor l_prc{ s };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    analyze::context anal_ctx{ *this };
    analyze::processor anal_prc{ *this, p_prc.begin(), p_prc.end() };
    evaluate::context eval_ctx{  *this };

    runtime::object_ptr result;
    for(auto anal_result(anal_prc.begin(anal_ctx)); anal_result != anal_prc.end(anal_ctx); ++anal_result)
    /* TODO: Codegen and JIT. */
    { result = eval_ctx.eval(anal_result->expect_ok().unwrap()); }
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

  result<var_ptr, std::string> context::intern_var(obj::symbol_ptr const &qualified_sym)
  { return intern_var(qualified_sym->ns, qualified_sym->name); }
  result<var_ptr, std::string> context::intern_var(detail::string_type const &ns, detail::string_type const &name)
  {
    auto const name_sym(runtime::obj::symbol::create(name));
    auto locked_namespaces(namespaces.ulock());
    auto const found_ns(locked_namespaces->find(runtime::obj::symbol::create(ns)));
    if(found_ns == locked_namespaces->end())
    { return err("can't intern var; namespace doesn't exist"); }

    auto locked_vars(found_ns->second->vars.ulock());
    auto const found_var(locked_vars->find(name_sym));
    if(found_var != locked_vars->end())
    { return ok(found_var->second); }

    auto const locked_vars_w(locked_vars.moveFromUpgradeToWrite());
    auto const ns_res(locked_vars_w->insert({name_sym, var::create(found_ns->second, name_sym)}));
    return ok(ns_res.first->second);
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
