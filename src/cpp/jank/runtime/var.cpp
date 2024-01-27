#include <fmt/compile.h>

#include <jank/runtime/var.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime
{
  /* NOTE: We default to nil, rather than a special unbound type. */
  var::static_object(ns_ptr const &n, obj::symbol_ptr const &name)
    : n{ n }
    , name{ name }
    , root{ obj::nil::nil_const() }
  {
  }

  var::static_object(ns_ptr const &n, obj::symbol_ptr const &name, object_ptr const root)
    : n{ n }
    , name{ name }
    , root{ root }
  {
  }

  var::static_object(ns_ptr const &n,
                     obj::symbol_ptr const &name,
                     object_ptr const root,
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
    if(o.type != object_type::var)
    {
      return false;
    }

    auto const v(expect_object<var>(&o));
    return n == v->n && name == v->name;
  }

  native_bool var::equal(var const &v) const
  {
    return n == v.n && name == v.name;
  }

  void to_string_impl(ns_ptr const n, obj::symbol_ptr const &name, fmt::memory_buffer &buff)
  {
    format_to(std::back_inserter(buff), FMT_COMPILE("#'{}/{}"), n->name->name, name->name);
  }

  void var::to_string(fmt::memory_buffer &buff) const
  {
    to_string_impl(n, name, buff);
  }
  native_persistent_string var::to_string() const
  /* TODO: Maybe cache this. */
  {
    fmt::memory_buffer buff;
    to_string_impl(n, name, buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_hash var::to_hash() const
  {
    if(hash)
    {
      return hash;
    }

    return hash = hash::combine(n->to_hash(), name->to_hash());
  }

  object_ptr var::with_meta(object_ptr const m)
  {
    meta = behavior::detail::validate_meta(m);
    return this;
  }

  object_ptr var::get_root() const
  {
    profile::timer timer{ "var get_root" };
    return *root.rlock();
  }

  var_ptr var::bind_root(object_ptr const r)
  {
    profile::timer timer{ "var bind_root" };
    *root.wlock() = r;
    return this;
  }

  string_result<void> var::set(object_ptr const r) const
  {
    profile::timer timer{ "var set" };

    auto const binding(get_thread_binding());
    if(!binding)
    {
      return err(fmt::format("Cannot set non-thread-bound var: {}", to_string()));
    }
    else if(std::this_thread::get_id() != binding->thread_id)
    {
      return err(
        fmt::format("Cannot set {} from a thread different from that which bound it", to_string()));
    }

    binding->value = r;
    return ok();
  }

  var_ptr var::set_dynamic(native_bool const dyn)
  {
    dynamic.store(dyn);
    return this;
  }

  var_thread_binding_ptr var::get_thread_binding() const
  {
    if(!thread_bound.load())
    {
      return nullptr;
    }

    assert(n);
    auto &tbfs(n->rt_ctx.thread_binding_frames[&n->rt_ctx]);
    if(tbfs.empty())
    {
      return nullptr;
    }

    assert(tbfs.front().bindings);
    auto const found(tbfs.front().bindings->get_entry(this));
    if(found == obj::nil::nil_const())
    {
      return nullptr;
    }

    auto const ret(expect_object<obj::persistent_vector>(found)->data[1]);
    assert(ret);
    return expect_object<var_thread_binding>(ret);
  }

  object_ptr var::deref() const
  {
    auto const binding(get_thread_binding());
    if(binding)
    {
      assert(binding->value);
      return binding->value;
    }
    return *root.rlock();
  }

  var_ptr var::clone() const
  {
    return make_box<var>(n, name, get_root(), dynamic.load(), thread_bound.load());
  }

  var_thread_binding::static_object(object_ptr const value, std::thread::id const id)
    : value{ value }
    , thread_id{ id }
  {
  }

  native_bool var_thread_binding::equal(object const &o) const
  {
    return &base == &o;
  }

  native_persistent_string var_thread_binding::to_string() const
  {
    return runtime::detail::to_string(value);
  }

  void var_thread_binding::to_string(fmt::memory_buffer &buff) const
  {
    return runtime::detail::to_string(value, buff);
  }

  native_hash var_thread_binding::to_hash() const
  {
    return hash::visit(value);
  }
}
