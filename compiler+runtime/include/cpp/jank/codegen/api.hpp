#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/var.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/obj/persistent_hash_set.hpp>
#include <jank/runtime/obj/jit_function.hpp>
#include <jank/runtime/obj/jit_variadic_function.hpp>
#include <jank/runtime/obj/jit_closure.hpp>
#include <jank/runtime/obj/jit_variadic_closure.hpp>
#include <jank/runtime/obj/number.hpp>

jank::runtime::var_ref _jank_var(char const *sym);
jank::runtime::var_ref _jank_var_owned(char const *sym);

jank::runtime::obj::keyword_ref _jank_keyword(char const * const ns, char const * const name);

jank::runtime::obj::symbol_ref _jank_symbol(char const * const ns, char const * const name);
jank::runtime::obj::symbol_ref
_jank_symbol(char const * const meta, char const * const ns, char const * const name);

jank::runtime::obj::integer_ref _jank_int(jtl::i64 const i);
jank::runtime::obj::small_integer_ref _jank_small_int(jtl::i32 const i);

jank::runtime::obj::real_ref _jank_real(jtl::f64 const r);
jank::runtime::obj::small_real_ref _jank_small_real(jtl::f64 const r);

jank::runtime::obj::persistent_string_ref _jank_string();
jank::runtime::obj::persistent_string_ref _jank_string(char const * const s);

jank::runtime::obj::persistent_list_ref _jank_list(jank::u64 const elems, ...);
jank::runtime::obj::persistent_list_ref
_jank_list(char const * const meta, jank::u64 const elems, ...);

jank::runtime::obj::persistent_vector_ref _jank_vec(jank::u64 const elems, ...);
jank::runtime::obj::persistent_vector_ref
_jank_vec(char const * const meta, jank::u64 const elems, ...);

jank::runtime::obj::persistent_hash_set_ref _jank_hset(jank::u64 const elems, ...);
jank::runtime::obj::persistent_hash_set_ref
_jank_hset(char const * const meta, jank::u64 const elems, ...);

jank::runtime::obj::persistent_array_map_ref _jank_amap(jank::u64 const pairs, ...);
jank::runtime::obj::persistent_array_map_ref
_jank_amap(char const * const meta, jank::u64 const pairs, ...);

jank::runtime::obj::persistent_hash_map_ref _jank_hmap(jank::u64 const pairs, ...);
jank::runtime::obj::persistent_hash_map_ref
_jank_hmap(char const * const meta, jank::u64 const pairs, ...);

jank::runtime::obj::jit_function_ref _jank_fn(jank::runtime::callable_arity_flags const flags);
jank::runtime::obj::jit_variadic_function_ref
_jank_vfn(jank::runtime::callable_arity_flags const flags);
jank::runtime::obj::jit_closure_ref
_jank_closure(jank::runtime::callable_arity_flags const flags, void * const ctx);
jank::runtime::obj::jit_variadic_closure_ref
_jank_vclosure(jank::runtime::callable_arity_flags const flags, void * const ctx);

jank::runtime::object_ref _jank_eval_str(char const *edn);
void _jank_refer_global(char const *fully_qualified_sym, char const *renamed_sym);
