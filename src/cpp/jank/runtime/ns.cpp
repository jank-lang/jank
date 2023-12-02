#include <memory>

#include <jank/runtime/ns.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/string.hpp>

namespace jank::runtime
{
  ns::static_object(obj::symbol_ptr const &name, context const &c)
    : name{ name }
    , vars{ obj::persistent_hash_map::empty() }
    , aliases{ obj::persistent_hash_map::empty() }
    , rt_ctx{ c }
  { }

  result<void, native_string> ns::add_alias(obj::symbol_ptr const &sym, native_box<static_object> const &ns)
  {
    auto locked_aliases(aliases.wlock());
    auto const found((*locked_aliases)->data.find(sym));
    if(found && expect_object<var>(*found) != ns)
    { return err(fmt::format("Alias already bound to a different ns: {}", sym->to_string())); }

    *locked_aliases = make_box<obj::persistent_hash_map>((*locked_aliases)->data.set(sym, ns));
    return ok();
  }

  option<ns_ptr> ns::find_alias(obj::symbol_ptr const &sym) const
  {
    auto locked_aliases(aliases.rlock());
    auto const found((*locked_aliases)->data.find(sym));
    if(found)
    { return expect_object<ns>(*found); }
    return none;
  }

  result<void, native_string> ns::refer(obj::symbol_ptr const sym, var_ptr const var)
  {
    auto locked_vars(vars.wlock());
    /* TODO: Unqualify vars. */
    //assert(sym->ns == name->name);
    if(auto const found = (*locked_vars)->data.find(sym))
    {
      return err
      (
        fmt::format
        (
          "{} already refers to {} in ns {}",
          sym->to_string(),
          expect_object<runtime::var>(*found)->to_string(),
          to_string()
        )
      );
    }
    *locked_vars = make_box<obj::persistent_hash_map>((*locked_vars)->data.set(sym, var));
    return ok();
  }

  obj::persistent_hash_map_ptr ns::get_mappings() const
  {
    auto const locked_vars(vars.rlock());
    return *locked_vars;
  }

  native_bool ns::equal(object const &o) const
  {
    if(o.type != object_type::ns)
    { return false; }

    auto const v(expect_object<ns>(&o));
    return name == v->name;
  }
  native_string ns::to_string() const
  /* TODO: Maybe cache this. */
  { return name->to_string(); }
  void ns::to_string(fmt::memory_buffer &buff) const
  { name->to_string(buff); }
  native_integer ns::to_hash() const
  /* TODO: Cache this. */
  { return name->to_hash(); }

  bool ns::operator ==(ns const &rhs) const
  { return name == rhs.name; }

  ns_ptr ns::clone() const
  {
    auto ret(make_box<ns>(name, rt_ctx));
    *ret->vars.wlock() = *vars.rlock();
    *ret->aliases.wlock() = *aliases.rlock();
    return ret;
  }
}
