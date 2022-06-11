#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/string.hpp>
#include <jank/runtime/obj/number.hpp>

namespace jank::runtime
{
  context::context()
  {
    auto const core(intern_ns(obj::symbol::create("clojure.core")));
    auto const ns_sym(obj::symbol::create("*ns*"));
    auto const ns_res(core->vars.insert({ns_sym, var::create(core, ns_sym, core)}));
    current_ns = ns_res.first->second;

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
        current_ns->set_root(new_ns);
        return JANK_NIL;
      }
    );
    auto const in_ns_res(core->vars.insert({in_ns_sym, var::create(core, in_ns_sym, obj::function::create(in_ns_fn))}));
    in_ns = in_ns_res.first->second;

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
    core->vars.insert({println_sym, var::create(core, println_sym, obj::function::create(println_fn))});

    /* TODO: Remove this once it can be defined in jank. */
    auto const plus_sym(obj::symbol::create("+"));
    core->vars.insert({plus_sym, var::create(core, plus_sym, obj::function::create(&obj::_gen_plus_))});
  }

  option<var_ptr> context::find_var(obj::symbol_ptr const &sym)
  {
    if(!sym->ns.empty())
    {
      auto const ns(namespaces.find(runtime::obj::symbol::create(sym->ns)));
      if(ns == namespaces.end())
      { return none; }
      auto const var(ns->second->vars.find(runtime::obj::symbol::create(sym->name)));
      if(var == ns->second->vars.end())
      { return none; }

      return { var->second };
    }
    else
    {
      auto const &vars(current_ns->root->as_ns()->vars);
      auto const var(vars.find(sym));
      if(var == vars.end())
      { return none; }

      return { var->second };
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
    { return var.unwrap()->root; }

    /* TODO: Return val. */
    return find_local(sym);
  }

  void context::dump() const
  {
    std::cout << "context dump" << std::endl;
    for(auto p : namespaces)
    {
      std::cout << "  " << p.second->name->to_string() << std::endl;
      for(auto vp : p.second->vars)
      {
        std::cout << "    " << vp.second->to_string() << " = " << vp.second->root->to_string() << std::endl;
      }
    }
  }

  ns_ptr context::intern_ns(obj::symbol_ptr const &sym)
  {
    auto const found(namespaces.find(sym));
    if(found != namespaces.end())
    { return found->second; }
    auto const result(namespaces.emplace(sym, make_box<ns>(sym, *this)));
    return result.first->second;
  }
}
