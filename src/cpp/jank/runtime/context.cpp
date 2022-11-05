#include <exception>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/string.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/util.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/util/mapped_file.hpp>
#include <jank/codegen/processor.hpp>
#include <jank/jit/processor.hpp>

namespace jank::runtime
{
  context::context()
  {
    auto &t_state(get_thread_state());
    auto const core(intern_ns(obj::symbol::create("clojure.core")));
    {
      auto const locked_core_vars(core->vars.wlock());
      auto const ns_sym(obj::symbol::create("clojure.core/*ns*"));
      auto const ns_res(locked_core_vars->insert({ns_sym, var::create(core, ns_sym, core)}));
      t_state.current_ns = ns_res.first->second;
    }

    auto const in_ns_sym(obj::symbol::create("clojure.core/in-ns"));
    std::function<object_ptr (object_ptr const&)> in_ns_fn
    (
      [this](object_ptr const &sym)
      {
        auto const * const s(sym->as_symbol());
        if(!s)
        {
          /* TODO: throw?. */
          return JANK_NIL;
        }
        auto const typed_sym(boost::static_pointer_cast<obj::symbol>(sym));
        auto const new_ns(intern_ns(typed_sym));
        get_thread_state().current_ns->set_root(new_ns);
        return JANK_NIL;
      }
    );
    auto in_ns_var(intern_var(in_ns_sym).expect_ok());
    in_ns_var->set_root(obj::function::create(in_ns_fn));
    t_state.in_ns = in_ns_var;

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
    intern_var(println_sym).expect_ok()->set_root(obj::function::create(println_fn));

    /* TODO: Remove this once it can be defined in jank. */
    auto const assert_sym(obj::symbol::create("clojure.core/assert"));
    std::function<object_ptr (object_ptr const&)> assert_fn
    (
      [](object_ptr const &o)
      {
        if(!o || !detail::truthy(o))
        { throw std::runtime_error{ "assertion failed" }; }
        return JANK_NIL;
      }
    );
    intern_var(assert_sym).expect_ok()->set_root(obj::function::create(assert_fn));

    /* TODO: Remove this once it can be defined in jank. */
    auto const plus_sym(obj::symbol::create("clojure.core/+"));
    intern_var(plus_sym).expect_ok()->set_root(obj::function::create(&obj::_gen_plus_));

    /* TODO: Remove this once it can be defined in jank. */
    auto const equal_sym(obj::symbol::create("clojure.core/="));
    intern_var(equal_sym).expect_ok()->set_root(obj::function::create(&_gen_equal_));
  }

  option<var_ptr> context::find_var(obj::symbol_ptr const &sym)
  {
    if(!sym->ns.empty())
    {
      /* TODO: This is the issue. Diff it with intern_var. */
      ns_ptr ns;
      {
        auto const locked_namespaces(namespaces.rlock());
        auto const found(locked_namespaces->find(runtime::obj::symbol::create("", sym->ns)));
        if(found == locked_namespaces->end())
        { return none; }
        ns = found->second;
      }

      {
        auto const locked_vars(ns->vars.rlock());
        auto const found(locked_vars->find(sym));
        if(found == locked_vars->end())
        { return none; }

        return { found->second };
      }
    }
    else
    {
      auto const t_state(get_thread_state());
      auto const * const current_ns(t_state.current_ns->get_root()->as_ns());
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

  void context::eval_prelude(analyze::context &an_ctx)
  {
    /* TODO: Know the location of this in any installation. */
    eval_file("src/jank/clojure/core.jank", an_ctx);
  }

  object_ptr context::eval_file(std::string_view const &path, analyze::context &an_ctx)
  {
    auto const file(util::map_file(path));
    return eval_string({ file.expect_ok().head, file.expect_ok().size }, an_ctx);
  }

  object_ptr context::eval_string(std::string_view const &code, analyze::context &an_ctx)
  {
    read::lex::processor l_prc{ code };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };
    jank::analyze::processor an_prc
    { *this, p_prc.begin(), p_prc.end() };
    codegen::processor cg_prc
    {
      *this,
      an_ctx,
      an_prc.result(an_ctx).expect_ok_move().unwrap()
    };

    jit::processor jit_prc;
    return jit_prc.eval(*this, cg_prc).expect_ok().unwrap();
  }

  void context::dump() const
  {
    std::cout << "context dump" << std::endl;
    for(auto const &p : *namespaces.rlock())
    {
      std::cout << "  " << p.second->name->to_string() << std::endl;
      for(auto const &vp : *p.second->vars.rlock())
      {
        if(vp.second->get_root() == nullptr)
        { std::cout << "    " << vp.second->to_string() << " = nil" << std::endl; }
        else
        { std::cout << "    " << vp.second->to_string() << " = " << vp.second->get_root()->to_string() << std::endl; }
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

  result<var_ptr, std::string> context::intern_var(detail::string_type const &ns, detail::string_type const &name)
  { return intern_var(runtime::obj::symbol::create(ns, name)); }
  result<var_ptr, std::string> context::intern_var(obj::symbol_ptr const &qualified_sym)
  {
    assert(!qualified_sym->ns.empty());
    auto locked_namespaces(namespaces.ulock());
    auto const found_ns(locked_namespaces->find(runtime::obj::symbol::create(qualified_sym->ns)));
    if(found_ns == locked_namespaces->end())
    { return err("can't intern var; namespace doesn't exist"); }

    auto locked_vars(found_ns->second->vars.ulock());
    auto const found_var(locked_vars->find(qualified_sym));
    if(found_var != locked_vars->end())
    { return ok(found_var->second); }

    auto const locked_vars_w(locked_vars.moveFromUpgradeToWrite());
    auto const ns_res(locked_vars_w->insert({qualified_sym, var::create(found_ns->second, qualified_sym)}));
    return ok(ns_res.first->second);
  }

  obj::keyword_ptr context::intern_keyword(obj::symbol const &sym, bool const resolved)
  { return intern_keyword(sym.ns, sym.name, resolved); }
  obj::keyword_ptr context::intern_keyword(std::string_view const &ns, std::string_view const &name, bool resolved)
  {
    obj::symbol sym{ ns, name };
    if(!resolved)
    {
      /* The ns will be an ns alias. */
      if(!ns.empty())
      { throw "unimplemented: auto-resolved ns aliases"; }
      else
      {
        auto const t_state(get_thread_state());
        auto const * const current_ns(t_state.current_ns->get_root()->as_ns());
        sym.ns = current_ns->name->name;
        resolved = true;
      }
    }

    auto locked_keywords(keywords.ulock());
    auto const found(locked_keywords->find(sym));
    if(found != locked_keywords->end())
    { return found->second; }

    auto const locked_keywords_w(locked_keywords.moveFromUpgradeToWrite());
    auto const res(locked_keywords_w->insert({sym, obj::keyword::create(sym, resolved)}));
    return res.first->second;
  }

  context::thread_state::thread_state(context &ctx)
    : rt_ctx{ ctx }
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
