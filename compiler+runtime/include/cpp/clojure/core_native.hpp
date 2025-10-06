#pragma once

#include <jank/c_api.h>
#include <jank/runtime/object.hpp>

extern "C" jank_object_ref jank_load_clojure_core_native();

namespace clojure::core_native
{
  using namespace jank::runtime;

  object_ref is_var(object_ref o);
  object_ref var_get(object_ref o);
  object_ref alter_var_root(object_ref o, object_ref fn, object_ref args);
  object_ref is_var_bound(object_ref o);
  object_ref is_var_thread_bound(object_ref o);
  object_ref var_bind_root(object_ref v, object_ref o);
  object_ref var_get_root(object_ref o);
  object_ref intern_var(object_ref sym);

  object_ref ns_unalias(object_ref current_ns, object_ref alias);
  object_ref ns_unmap(object_ref current_ns, object_ref sym);
  object_ref in_ns(object_ref sym);
  object_ref intern_ns(object_ref sym);
  object_ref find_ns(object_ref sym);
  object_ref find_var(object_ref sym);
  object_ref remove_ns(object_ref sym);
  object_ref is_ns(object_ref ns_or_sym);
  object_ref ns_name(object_ref ns);
  object_ref ns_map(object_ref ns);
  object_ref var_ns(object_ref v);
  object_ref ns_resolve(object_ref ns, object_ref sym);
  object_ref alias(object_ref current_ns, object_ref remote_ns, object_ref alias);
  object_ref refer(object_ref current_ns, object_ref sym, object_ref var);
  object_ref load_module(object_ref path);
  object_ref compile(object_ref path);

  object_ref not_(object_ref o);

  object_ref is_fn(object_ref o);
  object_ref is_multi_fn(object_ref o);
  object_ref
  multi_fn(object_ref name, object_ref dispatch_fn, object_ref default_, object_ref hierarchy);
  object_ref defmethod(object_ref multifn, object_ref dispatch_val, object_ref fn);
  object_ref remove_all_methods(object_ref multifn);
  object_ref remove_method(object_ref multifn, object_ref dispatch_val);
  object_ref prefer_method(object_ref multifn, object_ref dispatch_val_x, object_ref ispatch_val_y);
  object_ref methods(object_ref multifn);
  object_ref get_method(object_ref multifn, object_ref dispatch_val);
  object_ref prefers(object_ref multifn);

  object_ref sleep(object_ref ms);
  object_ref current_time();

  object_ref eval(object_ref expr);
  object_ref read_string(object_ref /* opts */, object_ref str);

  object_ref lazy_seq(object_ref o);

  object_ref hash_unordered(object_ref coll);
  object_ref jank_version();

  object_ref delay(object_ref fn);
}
