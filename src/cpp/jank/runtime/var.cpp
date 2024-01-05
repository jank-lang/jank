#include <forward_list>

#include <fmt/compile.h>

#include <jank/runtime/var.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime
{
  struct thread_binding_frame
  { obj::persistent_hash_map_ptr bindings{}; };

  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
  static thread_local std::forward_list<thread_binding_frame> thread_binding_frames;

  string_result<void> push_thread_bindings(obj::persistent_hash_map_ptr const bindings)
  {
    thread_binding_frame frame;
    obj::persistent_hash_map_ptr new_thread_bindings{};
    if(!thread_binding_frames.empty()) [[unlikely]]
    { new_thread_bindings = thread_binding_frames.front().bindings; }

    auto const thread_id(std::this_thread::get_id());

    for(auto it(bindings->fresh_seq()); it != nullptr; it = it->next_in_place())
    {
      auto const entry(it->first());
      auto const var(expect_object<var>(entry->data[0]));
      if(!var->dynamic.load()) [[unlikely]]
      { return err(fmt::format("Can't dynamically bind non-dynamic var: {}", var->to_string())); }
      var->thread_bound.store(true);
      new_thread_bindings = new_thread_bindings->assoc(var, make_box<var_thread_binding>(entry->data[1], thread_id));
    }

    thread_binding_frames.push_front(std::move(frame));
    return ok();
  }

  string_result<void> pop_thread_bindings()
  {
    if(thread_binding_frames.empty()) [[unlikely]]
    { return err("Mismatched thread binding pop"); }

    thread_binding_frames.pop_front();

    return ok();
  }

  option<thread_binding_frame> current_thread_binding_frame()
  {
    if(thread_binding_frames.empty()) [[likely]]
    { return none; }
    return thread_binding_frames.front();
  }

  /* NOTE: We default to nil, rather than a special unbound type. */
  var::static_object(ns_ptr const &n, obj::symbol_ptr const &s)
    : n{ n }, name{ s }, root{ obj::nil::nil_const() }
  { }
  var::static_object(ns_ptr const &n, obj::symbol_ptr const &s, object_ptr o)
    : n{ n }, name{ s }, root{ o }
  { }

  native_bool var::equal(object const &o) const
  {
    if(o.type != object_type::var)
    { return false; }

    auto const v(expect_object<var>(&o));
    return n == v->n && name == v->name;
  }

  native_bool var::equal(var const &v) const
  { return n == v.n && name == v.name; }

  void to_string_impl(ns_ptr const n, obj::symbol_ptr const &name, fmt::memory_buffer &buff)
  { format_to(std::back_inserter(buff), FMT_COMPILE("#'{}/{}"), n->name->name, name->name); }
  void var::to_string(fmt::memory_buffer &buff) const
  { to_string_impl(n, name, buff); }
  native_persistent_string var::to_string() const
  /* TODO: Maybe cache this. */
  {
    fmt::memory_buffer buff;
    to_string_impl(n, name, buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }
  native_integer var::to_hash() const
  /* TODO: Cache this. */
  { return detail::hash_combine(n->name->to_hash(), name->to_hash()); }

  object_ptr var::with_meta(object_ptr const m)
  {
    meta = behavior::detail::validate_meta(m);
    return this;
  }

  object_ptr var::get_root() const
  {
    profile::timer timer{ "var get_root" };
    if(!thread_bound.load())
    { return *root.rlock(); }

    return deref();
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
    { return err(fmt::format("Cannot set non-thread-bound var: {}", to_string())); }
    else if(std::this_thread::get_id() != binding->thread_id)
    { return err(fmt::format("Cannot set {} from a thread different from that which bound it", to_string())); }

    binding->value = r;
    return ok();
  }

  var_thread_binding_ptr var::get_thread_binding() const
  {
    if(!thread_bound.load() || thread_binding_frames.empty())
    { return nullptr; }

    auto const found(thread_binding_frames.front().bindings->get_entry(this));
    if(found == obj::nil::nil_const())
    { return nullptr; }

    return expect_object<var_thread_binding>(expect_object<obj::vector>(found)->data[1]);
  }

  object_ptr var::deref() const
  {
    auto const binding(get_thread_binding());
    if(binding)
    { return binding; }
    return *root.rlock();
  }

  var_ptr var::clone() const
  { return make_box<var>(n, name, get_root()); }

  var_thread_binding::static_object(object_ptr const value, std::thread::id const id)
    : value{ value }, thread_id{ id }
  { }

  native_bool var_thread_binding::equal(object const &o) const
  { return &base == &o; }

  native_persistent_string var_thread_binding::to_string() const
  { return runtime::detail::to_string(value); }

  void var_thread_binding::to_string(fmt::memory_buffer &buff) const
  { return runtime::detail::to_string(value, buff); }

  native_integer var_thread_binding::to_hash() const
  { return runtime::detail::to_hash(value); }
}
