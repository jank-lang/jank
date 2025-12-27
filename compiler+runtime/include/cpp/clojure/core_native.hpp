#pragma once

#include <jank/c_api.h>
#include <jank/runtime/object.hpp>

extern "C" void jank_load_clojure_core_native();

namespace clojure::core_native
{
  using namespace jank::runtime;

  object_ref is_var(object_ref const o);
  object_ref var_get(object_ref const o);
  object_ref alter_var_root(object_ref const o, object_ref const fn, object_ref const args);
  object_ref is_var_bound(object_ref const o);
  object_ref is_var_thread_bound(object_ref const o);
  object_ref var_bind_root(object_ref const v, object_ref const o);
  object_ref var_get_root(object_ref const o);
  object_ref intern_var(object_ref const sym);

  object_ref ns_unalias(object_ref const current_ns, object_ref const alias);
  object_ref ns_unmap(object_ref const current_ns, object_ref const sym);
  object_ref in_ns(object_ref const sym);
  object_ref intern_ns(object_ref const sym);
  object_ref find_ns(object_ref const sym);
  object_ref find_var(object_ref const sym);
  object_ref remove_ns(object_ref const sym);
  object_ref is_ns(object_ref const ns_or_sym);
  object_ref ns_name(object_ref const ns);
  object_ref ns_map(object_ref const ns);
  object_ref var_ns(object_ref const v);
  object_ref ns_resolve(object_ref const ns, object_ref const sym);
  object_ref alias(object_ref const current_ns, object_ref const remote_ns, object_ref const alias);
  object_ref refer(object_ref const current_ns, object_ref const sym, object_ref const var);
  object_ref load_module(object_ref const path);
  object_ref compile(object_ref const path);

  object_ref not_(object_ref const o);

  object_ref is_fn(object_ref const o);
  object_ref is_multi_fn(object_ref const o);
  object_ref multi_fn(object_ref const name,
                      object_ref const dispatch_fn,
                      object_ref const default_,
                      object_ref const hierarchy);
  object_ref
  defmethod(object_ref const multifn, object_ref const dispatch_val, object_ref const fn);
  object_ref remove_all_methods(object_ref const multifn);
  object_ref remove_method(object_ref const multifn, object_ref const dispatch_val);
  object_ref prefer_method(object_ref const multifn,
                           object_ref const dispatch_val_x,
                           object_ref const ispatch_val_y);
  object_ref methods(object_ref const multifn);
  object_ref get_method(object_ref const multifn, object_ref const dispatch_val);
  object_ref prefers(object_ref const multifn);

  object_ref sleep(object_ref const ms);
  object_ref current_time();

  object_ref eval(object_ref const expr);
  object_ref read_string(object_ref const /* opts */, object_ref const str);

  object_ref lazy_seq(object_ref const o);

  object_ref hash_unordered(object_ref const coll);
  object_ref jank_version();

  object_ref delay(object_ref const fn);
}
