#pragma once

#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/core/math.hpp>

namespace jank::runtime
{
  /* TODO: Header for this, with sequence equality fns. */
  native_bool equal(object_ptr lhs, object_ptr rhs);
  native_integer compare(object_ptr, object_ptr);
  native_bool is_identical(object_ptr lhs, object_ptr rhs);

  native_persistent_string type(object_ptr o);
  native_bool is_nil(object_ptr o);
  native_bool is_true(object_ptr o);
  native_bool is_false(object_ptr o);
  native_bool is_some(object_ptr o);
  native_bool is_string(object_ptr o);
  native_bool is_char(object_ptr o);

  native_bool is_symbol(object_ptr o);
  native_bool is_simple_symbol(object_ptr o);
  native_bool is_qualified_symbol(object_ptr o);

  object_ptr print(object_ptr args);
  object_ptr println(object_ptr args);
  object_ptr pr(object_ptr args);
  object_ptr prn(object_ptr args);

  object_ptr meta(object_ptr m);
  object_ptr with_meta(object_ptr o, object_ptr m);
  object_ptr reset_meta(object_ptr o, object_ptr m);

  obj::persistent_string_ptr subs(object_ptr s, object_ptr start);
  obj::persistent_string_ptr subs(object_ptr s, object_ptr start, object_ptr end);
  native_integer first_index_of(object_ptr s, object_ptr m);
  native_integer last_index_of(object_ptr s, object_ptr m);

  native_bool is_named(object_ptr o);
  native_persistent_string name(object_ptr o);
  object_ptr namespace_(object_ptr o);

  object_ptr keyword(object_ptr ns, object_ptr name);
  native_bool is_keyword(object_ptr o);
  native_bool is_simple_keyword(object_ptr o);
  native_bool is_qualified_keyword(object_ptr o);

  native_bool is_callable(object_ptr o);

  native_hash to_hash(object_ptr o);

  object_ptr macroexpand1(object_ptr o);
  object_ptr macroexpand(object_ptr o);

  object_ptr gensym(object_ptr o);

  object_ptr atom(object_ptr o);
  object_ptr deref(object_ptr o);
  object_ptr swap_atom(object_ptr atom, object_ptr fn);
  object_ptr swap_atom(object_ptr atom, object_ptr fn, object_ptr a1);
  object_ptr swap_atom(object_ptr atom, object_ptr fn, object_ptr a1, object_ptr a2);
  object_ptr
  swap_atom(object_ptr atom, object_ptr fn, object_ptr a1, object_ptr a2, object_ptr rest);
  object_ptr swap_vals(object_ptr atom, object_ptr fn);
  object_ptr swap_vals(object_ptr atom, object_ptr fn, object_ptr a1);
  object_ptr swap_vals(object_ptr atom, object_ptr fn, object_ptr a1, object_ptr a2);
  object_ptr
  swap_vals(object_ptr atom, object_ptr fn, object_ptr a1, object_ptr a2, object_ptr rest);
  object_ptr compare_and_set(object_ptr atom, object_ptr old_val, object_ptr new_val);
  object_ptr reset(object_ptr atom, object_ptr new_val);
  object_ptr reset_vals(object_ptr atom, object_ptr new_val);

  object_ptr volatile_(object_ptr o);
  native_bool is_volatile(object_ptr o);
  object_ptr vswap(object_ptr v, object_ptr fn, object_ptr args);
  object_ptr vreset(object_ptr v, object_ptr new_val);

  void push_thread_bindings(object_ptr o);
  void pop_thread_bindings();
  object_ptr get_thread_bindings();

  object_ptr force(object_ptr o);

  object_ptr tagged_literal(object_ptr tag, object_ptr form);
  native_bool is_tagged_literal(object_ptr o);
}
