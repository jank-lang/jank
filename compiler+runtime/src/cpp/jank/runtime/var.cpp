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
  var::var(ns_ref const &n, obj::symbol_ref const &name)
    : n{ n }
    , name{ name }
    , root{ make_box<var_unbound_root>(this) }
  {
  }

  var::var(ns_ref const &n, obj::symbol_ref const &name, object_ref const root)
    : n{ n }
    , name{ name }
    , root{ root }
  {
  }

  var::var(ns_ref const &n,
           obj::symbol_ref const &name,
           object_ref const root,
           native_bool const dynamic,
           native_bool const thread_bound)
    : n{ n }
    , name{ name }
    , root{ root }
    , dynamic{ dynamic }
    , thread_bound{ thread_bound }
  {
  }

  native_bool var::equal(object const &o) const
  {
    auto const v(dyn_cast<var>(&o));
    if(!v)
    {
      return false;
    }
    return n == v->n && name == v->name;
  }

  native_bool var::equal(var const &v) const
  {
    return n == v.n && name == v.name;
  }

  static void
  to_string_impl(ns_ref const n, obj::symbol_ref const &name, util::string_builder &buff)
  {
    buff("#'")(n->name->name)('/')(name->name);
  }

  void var::to_string(util::string_builder &buff) const
  {
    to_string_impl(n, name, buff);
  }
  jtl::immutable_string var::to_string() const
  /* TODO: Maybe cache this. */
  {
    util::string_builder buff;
    to_string_impl(n, name, buff);
    return buff.release();
  }

  jtl::immutable_string var::to_code_string() const
  {
    return to_string();
  }

  native_hash var::to_hash() const
  {
    if(hash)
    {
      return hash;
    }

    return hash = hash::combine(n->to_hash(), name->to_hash());
  }

  var_ref var::with_meta(object_ref const m)
  {
    meta = behavior::detail::validate_meta(m);
    return this;
  }

  native_bool var::is_bound() const
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
    if(!binding)
    {
      return err(util::format("Cannot set non-thread-bound var: {}", to_string()));
    }
    else if(std::this_thread::get_id() != binding->thread_id)
    {
      return err(util::format("Cannot set {} from a thread different from that which bound it",
                              to_string()));
    }

    binding->value = r;
    return ok();
  }

  var_ref var::set_dynamic(native_bool const dyn)
  {
    dynamic.store(dyn);
    return this;
  }

  var_thread_binding_ref var::get_thread_binding() const
  {
    if(!thread_bound.load())
    {
      return {};
    }

    auto &tbfs(n->rt_ctx.thread_binding_frames[&n->rt_ctx]);
    if(tbfs.empty())
    {
      return {};
    }

    auto const found(tbfs.front().bindings->get_entry(this));
    if(found == obj::nil::nil_const())
    {
      return {};
    }

    auto const ret(expect_object<obj::persistent_vector>(found)->data[1]);
    return expect_object<var_thread_binding>(ret);
  }

  object_ref var::deref() const
  {
    auto const binding(get_thread_binding());
    if(binding)
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
    : value{ value }
    , thread_id{ id }
  {
  }

  native_bool var_thread_binding::equal(object const &o) const
  {
    return &base == &o;
  }

  jtl::immutable_string var_thread_binding::to_string() const
  {
    return runtime::to_string(value);
  }

  jtl::immutable_string var_thread_binding::to_code_string() const
  {
    return var_thread_binding::to_string();
  }

  void var_thread_binding::to_string(util::string_builder &buff) const
  {
    runtime::to_string(value, buff);
  }

  native_hash var_thread_binding::to_hash() const
  {
    return hash::visit(value.get());
  }

  var_unbound_root::var_unbound_root(var_ref const var)
    : var{ var }
  {
  }

  native_bool var_unbound_root::equal(object const &o) const
  {
    return &base == &o;
  }

  jtl::immutable_string var_unbound_root::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void var_unbound_root::to_string(util::string_builder &buff) const
  {
    util::format_to(buff, "unbound@{} for var {}", &base, var->to_string());
  }

  jtl::immutable_string var_unbound_root::to_code_string() const
  {
    return var_unbound_root::to_string();
  }

  native_hash var_unbound_root::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }
}
