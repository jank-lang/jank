#include <clojure/core_native.hpp>
#include <jank/runtime/convert/function.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/call.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/error/runtime.hpp>
#include <jank/util/fmt/print.hpp>

namespace clojure::core_native
{
  using namespace jank;
  using namespace jank::runtime;

  object_ref not_(object_ref const o)
  {
    if(runtime::is_nil(o))
    {
      return jank_true;
    }
    return make_box(runtime::is_false(o));
  }

  object_ref lazy_seq(object_ref const o)
  {
    return make_box<obj::lazy_sequence>(o);
  }

  object_ref is_var(object_ref const o)
  {
    return make_box(o.get_type() == object_type::var);
  }

  object_ref var_get(object_ref const o)
  {
    return try_object<var>(o)->deref();
  }

  object_ref intern_var(object_ref const sym)
  {
    return __rt_ctx->intern_var(try_object<obj::symbol>(sym)).expect_ok();
  }

  object_ref var_get_root(object_ref const o)
  {
    return try_object<var>(o)->get_root();
  }

  object_ref var_bind_root(object_ref const v, object_ref const o)
  {
    return try_object<var>(v)->bind_root(o);
  }

  object_ref alter_var_root(object_ref const o, object_ref const fn, object_ref const args)
  {
    return try_object<var>(o)->alter_root(fn, args);
  }

  object_ref is_var_bound(object_ref const o)
  {
    return make_box(try_object<runtime::var>(o)->is_bound());
  }

  object_ref is_var_thread_bound(object_ref const o)
  {
    return make_box(try_object<runtime::var>(o)->get_thread_binding().is_some());
  }

  object_ref delay(object_ref const fn)
  {
    return make_box<obj::delay>(fn);
  }

  object_ref is_fn(object_ref const o)
  {
    return make_box(o.get_type() == object_type::native_function_wrapper
                    || o.get_type() == object_type::jit_function
                    || o.get_type() == object_type::jit_variadic_function
                    || o.get_type() == object_type::jit_closure
                    || o.get_type() == object_type::jit_variadic_closure
                    || o.get_type() == object_type::deferred_cpp_function);
  }

  object_ref is_multi_fn(object_ref const o)
  {
    return make_box(o.get_type() == object_type::multi_function);
  }

  object_ref multi_fn(object_ref const name,
                      object_ref const dispatch_fn,
                      object_ref const default_,
                      object_ref const hierarchy)
  {
    return make_box<obj::multi_function>(name, dispatch_fn, default_, hierarchy);
  }

  object_ref defmethod(object_ref const multifn, object_ref const dispatch_val, object_ref const fn)
  {
    return try_object<obj::multi_function>(multifn)->add_method(dispatch_val, fn);
  }

  object_ref remove_all_methods(object_ref const multifn)
  {
    return try_object<obj::multi_function>(multifn)->reset();
  }

  object_ref remove_method(object_ref const multifn, object_ref const dispatch_val)
  {
    return try_object<obj::multi_function>(multifn)->remove_method(dispatch_val);
  }

  object_ref prefer_method(object_ref const multifn,
                           object_ref const dispatch_val_x,
                           object_ref const dispatch_val_y)
  {
    return try_object<obj::multi_function>(multifn)->prefer_method(dispatch_val_x, dispatch_val_y);
  }

  object_ref methods(object_ref const multifn)
  {
    return try_object<obj::multi_function>(multifn)->method_table;
  }

  object_ref get_method(object_ref const multifn, object_ref const dispatch_val)
  {
    return try_object<obj::multi_function>(multifn)->get_fn(dispatch_val);
  }

  object_ref prefers(object_ref const multifn)
  {
    return try_object<obj::multi_function>(multifn)->prefer_table;
  }

  object_ref sleep(object_ref const ms)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(to_int(ms)));
    return {};
  }

  object_ref current_time()
  {
    using namespace std::chrono;
    auto const t(high_resolution_clock::now());
    return make_box(duration_cast<nanoseconds>(t.time_since_epoch()).count());
  }

  object_ref in_ns(object_ref const sym)
  {
    __rt_ctx->current_ns_var->set(__rt_ctx->intern_ns(try_object<obj::symbol>(sym))).expect_ok();
    return {};
  }

  object_ref intern_ns(object_ref const sym)
  {
    return __rt_ctx->intern_ns(try_object<obj::symbol>(sym));
  }

  object_ref find_ns(object_ref const sym)
  {
    return __rt_ctx->find_ns(try_object<obj::symbol>(sym));
  }

  object_ref find_var(object_ref const sym)
  {
    return __rt_ctx->find_var(try_object<obj::symbol>(sym));
  }

  object_ref remove_ns(object_ref const sym)
  {
    return __rt_ctx->remove_ns(try_object<obj::symbol>(sym));
  }

  object_ref is_ns(object_ref const ns_or_sym)
  {
    return make_box(ns_or_sym.get_type() == object_type::ns);
  }

  object_ref ns_name(object_ref const ns)
  {
    return try_object<runtime::ns>(ns)->name;
  }

  object_ref ns_map(object_ref const ns)
  {
    return try_object<runtime::ns>(ns)->get_mappings();
  }

  object_ref var_ns(object_ref const v)
  {
    return try_object<runtime::var>(v)->n;
  }

  object_ref ns_resolve(object_ref const ns, object_ref const sym)
  {
    auto const n(try_object<runtime::ns>(ns));
    auto const found(n->find_var(try_object<obj::symbol>(sym)));
    return found;
  }

  object_ref alias(object_ref const current_ns, object_ref const remote_ns, object_ref const alias)
  {
    try_object<ns>(current_ns)
      ->add_alias(try_object<obj::symbol>(alias), try_object<ns>(remote_ns))
      .expect_ok();
    return {};
  }

  object_ref ns_unalias(object_ref const current_ns, object_ref const alias)
  {
    try_object<ns>(current_ns)->remove_alias(try_object<obj::symbol>(alias));
    return {};
  }

  object_ref ns_unmap(object_ref const current_ns, object_ref const sym)
  {
    try_object<ns>(current_ns)->unmap(try_object<obj::symbol>(sym)).expect_ok();
    return {};
  }

  object_ref refer(object_ref const current_ns, object_ref const sym, object_ref const var)
  {
    expect_object<runtime::ns>(current_ns)
      ->refer(try_object<obj::symbol>(sym), expect_object<runtime::var>(var))
      .expect_ok();
    return {};
  }

  object_ref refer_global(object_ref const sym)
  {
    auto const current_ns{ __rt_ctx->current_ns() };
    current_ns->refer_global(sym).expect_ok();
    return {};
  }

  object_ref rename_referred_globals(object_ref const rename_map)
  {
    if(is_empty(rename_map))
    {
      return {};
    }

    auto const current_ns{ __rt_ctx->current_ns() };
    current_ns->rename_referred_globals(rename_map).expect_ok();
    return {};
  }

  object_ref throw_runtime_invalid_referred_global_rename(jtl::immutable_string const &msg,
                                                          object_ref const rename,
                                                          object_ref const existing)
  {
    throw error::runtime_invalid_referred_global_rename(msg,
                                                        object_source(rename),
                                                        object_source(existing));
    return {};
  }

  object_ref load_module(object_ref const path)
  {
    __rt_ctx->load_module(runtime::to_string(path), module::origin::latest).expect_ok();
    return {};
  }

  object_ref compile(object_ref const path)
  {
    __rt_ctx->compile_module(runtime::to_string(path)).expect_ok();
    return {};
  }

  object_ref eval(object_ref const expr)
  {
    return __rt_ctx->eval(expr);
  }

  object_ref hash_unordered(object_ref const coll)
  {
    return make_box(hash::unordered(coll)).erase();
  }

  object_ref jank_version()
  {
    return make_box(JANK_VERSION);
  }
}

extern "C" void jank_load_clojure_core_native()
{
  using namespace jank;
  using namespace jank::runtime;
  using namespace clojure;

  auto const ns_name{ "clojure.core-native" };
  auto const ns(__rt_ctx->intern_ns(ns_name));

  /* Will not be required, once we implement this module in jank with
   * cpp interop. */
  __rt_ctx->module_loader.set_is_loaded(ns_name);

  auto const intern_fn([=](jtl::immutable_string const &name, auto const fn) {
    ns->intern_var(name)->bind_root(
      make_box<obj::native_function_wrapper>(convert_function(fn))
        ->with_meta(obj::persistent_hash_map::create_unique(std::make_pair(
          __rt_ctx->intern_keyword("name").expect_ok(),
          make_box(obj::symbol{ __rt_ctx->current_ns()->to_string(), name }.to_string())))));
  });

  intern_fn("load-module", &core_native::load_module);
}
