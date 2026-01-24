#include <jank/runtime/var.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/profile/time.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime
{
  var::var(ns_ref const n, obj::symbol_ref const name)
    : object{ obj_type }
    , n{ n }
    , name{ name }
    , root{ make_box<var_unbound_root>(this) }
  {
  }

  var::var(ns_ref const n, obj::symbol_ref const name, object_ref const root)
    : object{ obj_type }
    , n{ n }
    , name{ name }
    , root{ root }
  {
  }

  var::var(ns_ref const n,
           obj::symbol_ref const name,
           object_ref const root,
           bool const dynamic,
           bool const thread_bound)
    : object{ obj_type }
    , n{ n }
    , name{ name }
    , root{ root }
    , dynamic{ dynamic }
    , thread_bound{ thread_bound }
  {
  }

  bool var::equal(object const &o) const
  {
    auto const v(dyn_cast<var>(&o));
    if(v.is_nil())
    {
      return false;
    }
    return n == v->n && name == v->name;
  }

  bool var::equal(var const &v) const
  {
    return n == v.n && name == v.name;
  }

  static void to_string_impl(ns_ref const n, obj::symbol_ref const name, jtl::string_builder &buff)
  {
    buff("#'")(n->name->name)('/')(name->name);
  }

  void var::to_string(jtl::string_builder &buff) const
  {
    to_string_impl(n, name, buff);
  }
  jtl::immutable_string var::to_string() const
  /* TODO: Maybe cache this. */
  {
    jtl::string_builder buff;
    to_string_impl(n, name, buff);
    return buff.release();
  }

  jtl::immutable_string var::to_code_string() const
  {
    return to_string();
  }

  uhash var::to_hash() const
  {
    return hash::combine(n->to_hash(), name->to_hash());
  }

  bool var::is_bound() const
  {
    return deref()->type != object_type::var_unbound_root;
  }

  object_ref var::get_root() const
  {
    profile::timer const timer{ "var get_root" };
    return *root.rlock();
  }

  var_ref var::bind_root(object_ref const r)
  {
    profile::timer const timer{ "var bind_root" };
    *root.wlock() = r;
    return this;
  }

  object_ref var::alter_root(object_ref const f, object_ref const args)
  {
    auto locked_root(root.wlock());
    *locked_root = apply_to(f, cons(*locked_root, args));
    return *locked_root;
  }

  jtl::string_result<void> var::set(object_ref const r) const
  {
    profile::timer const timer{ "var set" };

    auto const binding(get_thread_binding());
    if(binding.is_nil())
    {
      return err(util::format("Cannot set non-thread-bound var: {}", to_string()));
    }
    else if(std::this_thread::get_id() != binding->thread_id)
    {
      return err(util::format("Cannot set {} from a thread different from that which bound it.",
                              to_string()));
    }

    binding->value = r;
    return ok();
  }

  var_ref var::set_dynamic(bool const dyn)
  {
    dynamic.store(dyn);
    return this;
  }

  obj::symbol_ref var::to_qualified_symbol() const
  {
    return make_box<runtime::obj::symbol>(n->name->name, name->name);
  }

  var_thread_binding_ref var::get_thread_binding() const
  {
    if(!thread_bound.load())
    {
      return {};
    }

    auto &tbfs(runtime::context::thread_binding_frames);
    if(tbfs.empty())
    {
      return {};
    }

    auto const found(tbfs.front().bindings->get_entry(this));
    if(found.is_nil())
    {
      return {};
    }

    auto const ret(expect_object<obj::persistent_vector>(found)->data[1]);
    return expect_object<var_thread_binding>(ret);
  }

  object_ref var::deref() const
  {
    auto const binding(get_thread_binding());
    if(binding.is_some())
    {
      return binding->value;
    }
    return *root.rlock();
  }

  var_ref var::clone() const
  {
    return make_box<var>(n, name, get_root(), dynamic.load(), thread_bound.load());
  }

  var_thread_binding::var_thread_binding(object_ref const value, std::thread::id const id)
    : object{ obj_type }
    , value{ value }
    , thread_id{ id }
  {
  }

  jtl::immutable_string var_thread_binding::to_string() const
  {
    return runtime::to_string(value);
  }

  jtl::immutable_string var_thread_binding::to_code_string() const
  {
    return var_thread_binding::to_string();
  }

  void var_thread_binding::to_string(jtl::string_builder &buff) const
  {
    runtime::to_string(value, buff);
  }

  uhash var_thread_binding::to_hash() const
  {
    return hash::visit(value.get());
  }

  var_unbound_root::var_unbound_root(var_ref const var)
    : object{ obj_type }
    , var{ var }
  {
  }

  void var_unbound_root::to_string(jtl::string_builder &buff) const
  {
    util::format_to(buff, "unbound@{} for var {}", this, var->to_string());
  }
}

namespace std
{
  size_t hash<jank::runtime::var>::operator()(jank::runtime::var const &o) const noexcept
  {
    static auto hasher(std::hash<jank::runtime::obj::symbol>{});
    return hasher(*o.name);
  }

  size_t hash<jank::runtime::var_ref>::operator()(jank::runtime::var_ref const o) const noexcept
  {
    static auto hasher(std::hash<jank::runtime::obj::symbol>{});
    return hasher(*o->name);
  }

  bool equal_to<jank::runtime::var_ref>::operator()(jank::runtime::var_ref const lhs,
                                                    jank::runtime::var_ref const rhs) const noexcept
  {
    return lhs->equal(*rhs);
  }
}
