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

  object_ref to_unqualified_symbol(object_ref const o);
  object_ref to_qualified_symbol(object_ref const ns, object_ref const name);

  object_ref print(object_ref const args);
  object_ref println(object_ref const args);
  object_ref pr(object_ref const args);
  object_ref prn(object_ref const args);

  obj::persistent_string_ref subs(object_ref const s, object_ref const start);
  obj::persistent_string_ref subs(object_ref const s, object_ref const start, object_ref const end);
  i64 first_index_of(object_ref const s, object_ref const m);
  i64 last_index_of(object_ref const s, object_ref const m);

  bool is_named(object_ref const o);
  jtl::immutable_string name(object_ref const o);
  object_ref namespace_(object_ref const o);

  object_ref keyword(object_ref const ns, object_ref const name);
  bool is_keyword(object_ref const o);
  bool is_simple_keyword(object_ref const o);
  bool is_qualified_keyword(object_ref const o);

  bool is_callable(object_ref const o);

  uhash to_hash(object_ref const o);

  object_ref macroexpand1(object_ref const o);
  object_ref macroexpand(object_ref const o);

  object_ref gensym(object_ref const o);

  object_ref atom(object_ref const o);
  object_ref deref(object_ref const o);
  object_ref swap_atom(object_ref const atom, object_ref const fn);
  object_ref swap_atom(object_ref const atom, object_ref const fn, object_ref const a1);
  object_ref
  swap_atom(object_ref const atom, object_ref const fn, object_ref const a1, object_ref const a2);
  object_ref swap_atom(object_ref const atom,
                       object_ref const fn,
                       object_ref const a1,
                       object_ref const a2,
                       object_ref const rest);
  object_ref swap_vals(object_ref const atom, object_ref const fn);
  object_ref swap_vals(object_ref const atom, object_ref const fn, object_ref const a1);
  object_ref
  swap_vals(object_ref const atom, object_ref const fn, object_ref const a1, object_ref const a2);
  object_ref swap_vals(object_ref const atom,
                       object_ref const fn,
                       object_ref const a1,
                       object_ref const a2,
                       object_ref const rest);
  object_ref
  compare_and_set(object_ref const atom, object_ref const old_val, object_ref const new_val);
  object_ref reset(object_ref const atom, object_ref const new_val);
  object_ref reset_vals(object_ref const atom, object_ref const new_val);

  object_ref volatile_(object_ref const o);
  bool is_volatile(object_ref const o);
  object_ref vswap(object_ref const v, object_ref const fn);
  object_ref vswap(object_ref const v, object_ref const fn, object_ref const args);
  object_ref vreset(object_ref const v, object_ref const new_val);

  void push_thread_bindings(object_ref const o);
  void pop_thread_bindings();
  object_ref get_thread_bindings();

  object_ref force(object_ref const o);

  object_ref tagged_literal(object_ref const tag, object_ref const form);
  bool is_tagged_literal(object_ref const o);

  object_ref parse_uuid(object_ref const o);
  bool is_uuid(object_ref const o);
  object_ref random_uuid();

  bool is_inst(object_ref const o);
  i64 inst_ms(object_ref const o);

  object_ref re_pattern(object_ref const o);
  object_ref re_matcher(object_ref const re, object_ref const s);
  object_ref re_find(object_ref const m);
  object_ref re_groups(object_ref const m);
  object_ref re_matches(object_ref const re, object_ref const s);
  object_ref smatch_to_vector(std::smatch const &match_results);

  object_ref add_watch(object_ref const reference, object_ref const key, object_ref const fn);
  object_ref remove_watch(object_ref const reference, object_ref const key);
}
