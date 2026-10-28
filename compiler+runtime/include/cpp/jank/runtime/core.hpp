#pragma once

#include <regex>

/* TODO: Remove these so that people include only what they need. */
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/core/math.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using atom_ref = oref<struct atom>;
    using future_ref = oref<struct future>;
    using keyword_ref = oref<struct keyword>;
    using native_vector_sequence_ref = oref<struct native_vector_sequence>;
    using promise_ref = oref<struct promise>;
    using re_pattern_ref = oref<struct re_pattern>;
    using symbol_ref = oref<struct symbol>;
    using tagged_literal_ref = oref<struct tagged_literal>;
    using uuid_ref = oref<struct uuid>;
    using volatile_ref = oref<struct volatile_>;
    using exception_info_ref = oref<struct exception_info>;
  }

  bool not_(object_ref const o);

  jtl::immutable_string type(object_ref const o);
  bool is_nil(object_ref const o);
  bool is_true(object_ref const o);
  bool is_false(object_ref const o);
  bool is_some(object_ref const o);
  bool is_string(object_ref const o);
  bool is_char(object_ref const o);

  bool is_symbol(object_ref const o);
  bool is_simple_symbol(object_ref const o);
  bool is_qualified_symbol(object_ref const o);

  obj::symbol_ref to_unqualified_symbol(object_ref const o);
  obj::symbol_ref to_qualified_symbol(object_ref const ns, object_ref const name);

  object_ref print(object_ref const args);
  object_ref print1(object_ref const o);
  object_ref println(object_ref const args);
  object_ref pr(object_ref const args);
  object_ref prn(object_ref const args);
  jtl::immutable_string format(jtl::immutable_string const &format, object_ref const args);

  obj::persistent_string_ref subs(object_ref const s, object_ref const start);
  obj::persistent_string_ref subs(object_ref const s, object_ref const start, object_ref const end);
  i64 first_index_of(object_ref const s, object_ref const m);
  i64 last_index_of(object_ref const s, object_ref const m);

  bool is_named(object_ref const o);
  jtl::immutable_string name(object_ref const o);
  object_ref namespace_(object_ref const o);
  obj::native_vector_sequence_ref all_ns();

  obj::keyword_ref keyword(object_ref const ns, object_ref const name);
  bool is_keyword(object_ref const o);
  bool is_simple_keyword(object_ref const o);
  bool is_qualified_keyword(object_ref const o);

  bool is_fn(object_ref const o);
  bool is_callable(object_ref const o);
  bool is_multi_fn(object_ref const o);
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
                           object_ref const dispatch_val_y);
  object_ref methods(object_ref const multifn);
  object_ref get_method(object_ref const multifn, object_ref const dispatch_val);
  object_ref prefers(object_ref const multifn);

  uhash to_hash(object_ref const o);

  object_ref macroexpand1(object_ref const o);
  object_ref macroexpand(object_ref const o);

  obj::symbol_ref gensym(object_ref const o);

  obj::atom_ref atom(object_ref const o);
  object_ref deref(object_ref const o);
  bool is_realized(object_ref const o);
  object_ref swap_atom(obj::atom_ref const atom, object_ref const fn);
  object_ref swap_atom(obj::atom_ref const atom, object_ref const fn, object_ref const a1);
  object_ref swap_atom(obj::atom_ref const atom,
                       object_ref const fn,
                       object_ref const a1,
                       object_ref const a2);
  object_ref swap_atom(obj::atom_ref const atom,
                       object_ref const fn,
                       object_ref const a1,
                       object_ref const a2,
                       object_ref const rest);
  object_ref swap_vals(obj::atom_ref const atom, object_ref const fn);
  object_ref swap_vals(obj::atom_ref const atom, object_ref const fn, object_ref const a1);
  object_ref swap_vals(obj::atom_ref const atom,
                       object_ref const fn,
                       object_ref const a1,
                       object_ref const a2);
  object_ref swap_vals(obj::atom_ref const atom,
                       object_ref const fn,
                       object_ref const a1,
                       object_ref const a2,
                       object_ref const rest);
  object_ref
  compare_and_set(obj::atom_ref const atom, object_ref const old_val, object_ref const new_val);
  object_ref reset(obj::atom_ref const atom, object_ref const new_val);
  object_ref reset_vals(obj::atom_ref const atom, object_ref const new_val);

  object_ref volatile_(object_ref const o);
  bool is_volatile(object_ref const o);
  object_ref vswap(obj::volatile_ref const v, object_ref const fn);
  object_ref vswap(obj::volatile_ref const v, object_ref const fn, object_ref const args);
  object_ref vreset(obj::volatile_ref const v, object_ref const new_val);

  void push_thread_bindings(object_ref const o);
  void pop_thread_bindings();
  object_ref get_thread_bindings();

  bool is_var(object_ref const o);
  object_ref var_get(object_ref const o);
  object_ref intern_var(object_ref const sym);
  object_ref var_get_root(object_ref const o);
  object_ref var_bind_root(object_ref const v, object_ref const o);
  object_ref alter_var_root(object_ref const o, object_ref const fn, object_ref const args);
  bool is_var_bound(object_ref const o);
  bool is_var_thread_bound(object_ref const o);

  object_ref in_ns(object_ref const sym);
  object_ref intern_ns(object_ref const sym);
  object_ref find_ns(object_ref const sym);
  object_ref find_var(object_ref const sym);
  object_ref remove_ns(object_ref const sym);
  bool is_ns(object_ref const ns_or_sym);
  object_ref ns_name(object_ref const ns);
  object_ref ns_map(object_ref const ns);
  object_ref var_ns(object_ref const v);
  object_ref ns_resolve(object_ref const ns, object_ref const sym);
  object_ref alias(object_ref const current_ns, object_ref const remote_ns, object_ref const alias);
  object_ref ns_unalias(object_ref const current_ns, object_ref const alias);
  object_ref ns_unmap(object_ref const current_ns, object_ref const sym);
  object_ref refer(object_ref const current_ns, object_ref const sym, object_ref const var);
  object_ref refer_global(object_ref const sym);
  object_ref rename_referred_globals(object_ref const rename_map);

  obj::tagged_literal_ref tagged_literal(object_ref const tag, object_ref const form);
  bool is_tagged_literal(object_ref const o);

  object_ref parse_uuid(object_ref const o);
  bool is_uuid(object_ref const o);
  obj::uuid_ref random_uuid();

  bool is_inst(object_ref const o);
  i64 inst_ms(object_ref const o);

  obj::re_pattern_ref re_pattern(object_ref const o);
  object_ref re_matcher(object_ref const re, object_ref const s);
  object_ref re_find(object_ref const m);
  object_ref re_groups(object_ref const m);
  object_ref re_matches(object_ref const re, object_ref const s);
  object_ref smatch_to_vector(std::smatch const &match_results);

  void set_validator(object_ref reference, object_ref const validator_fn);
  object_ref get_validator(object_ref const reference);
  object_ref add_watch(object_ref reference, object_ref const key, object_ref const fn);
  object_ref remove_watch(object_ref reference, object_ref const key);

  obj::future_ref future(object_ref const fn);
  void cancel_future(obj::future_ref const future);
  bool is_future_cancelled(obj::future_ref const future);

  obj::delay_ref delay(object_ref const fn);
  object_ref force(object_ref const o);
  obj::promise_ref promise();

  object_ref sleep(object_ref const ms);
  object_ref current_time();

  object_ref read_string(object_ref const form_string, object_ref const opts);
  object_ref read_file(object_ref const file_path, object_ref const opts);

  obj::character_ref to_char(object_ref const x);

  obj::exception_info_ref ex_info(jtl::immutable_string const &message, object_ref const data);
  obj::exception_info_ref
  ex_info(jtl::immutable_string const &message, object_ref const data, object_ref const cause);

  jtl::immutable_string ex_message(object_ref const o);
  object_ref ex_data(object_ref const o);
  obj::exception_info_ref ex_cause(object_ref const o);
  obj::persistent_array_map_ref ex_map(object_ref const o);

  object_ref compile(object_ref const path);
  object_ref eval(object_ref const expr);
  jtl::u32 hash_unordered(object_ref const coll);
  jtl::immutable_string jank_version();
}
