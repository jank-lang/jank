#include <jank/runtime/context.hpp>
#include <jank/runtime/type/fn.hpp>
#include <jank/runtime/type/string.hpp>

namespace jank::runtime
{
  void context::initialize()
  {
    namespaces.clear();
    auto core(intern_ns(type::symbol::create("clojure.core")));
    auto ns_sym(type::symbol::create("*ns*"));
    auto ns_res(core->vars.insert({ns_sym, var::create(core, ns_sym, core)}));
    current_ns = ns_res.first->second;

    auto in_ns_sym(type::symbol::create("in-ns"));
    std::function<object_ptr (object_ptr const&)> in_ns_fn
    (
      [this](object_ptr const &sym)
      {
        type::symbol const * const s(sym->as_symbol());
        if(!s)
        {
          /* TODO: throw?. */
          return JANK_NIL;
        }
        auto typed_sym(boost::static_pointer_cast<type::symbol>(sym));
        auto new_ns(intern_ns(typed_sym));
        current_ns->set_root(new_ns);
        return JANK_NIL;
      }
    );
    auto in_ns_res(core->vars.insert({in_ns_sym, var::create(core, in_ns_sym, type::function::create(in_ns_fn))}));
    in_ns = in_ns_res.first->second;
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

  ns_ptr context::intern_ns(type::symbol_ptr const &sym)
  {
    auto const found(namespaces.find(sym));
    if(found != namespaces.end())
    { return found->second; }
    auto const result(namespaces.emplace(sym, make_box<ns>(sym, *this)));
    return result.first->second;
  }
}
