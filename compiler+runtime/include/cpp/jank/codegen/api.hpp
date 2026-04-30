#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/var.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/jit_function.hpp>
#include <jank/runtime/obj/jit_closure.hpp>
#include <jank/runtime/obj/number.hpp>

jank::runtime::var_ref _jank_var(char const *sym);
jank::runtime::var_ref _jank_var_owned(char const *sym);
jank::runtime::obj::keyword_ref _jank_keyword(char const * const ns, char const * const name);
jank::runtime::obj::symbol_ref _jank_symbol(char const * const ns, char const * const name);
jank::runtime::obj::integer_ref _jank_int(jtl::i64 const i);
jank::runtime::obj::small_integer_ref _jank_small_int(jtl::i64 const i);
jank::runtime::obj::real_ref _jank_real(jtl::f64 const r);
jank::runtime::obj::symbol_ref
_jank_symbol(jank::runtime::object_ref const meta, char const * const ns, char const * const name);
jank::runtime::obj::persistent_string_ref _jank_string(char const * const s);
jank::runtime::obj::jit_function_ref _jank_fn(jank::runtime::callable_arity_flags const flags);
jank::runtime::obj::jit_closure_ref
_jank_closure(jank::runtime::callable_arity_flags const flags, void * const ctx);
jank::runtime::object_ref _jank_read(char const *edn);
