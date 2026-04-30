#include <jank/codegen/api.hpp>
#include <jank/runtime/context.hpp>

jank::runtime::var_ref _jank_var(char const * const sym)
{
  return jank::runtime::__rt_ctx->intern_var(sym).expect_ok();
}

jank::runtime::var_ref _jank_var_owned(char const * const sym)
{
  return jank::runtime::__rt_ctx->intern_owned_var(sym).expect_ok();
}

jank::runtime::obj::keyword_ref _jank_keyword(char const * const ns, char const * const name)
{
  return jank::runtime::__rt_ctx->intern_keyword(ns, name, true).expect_ok();
}

jank::runtime::obj::symbol_ref _jank_symbol(char const * const ns, char const * const name)
{
  return jank::runtime::make_box<jank::runtime::obj::symbol>(
    jank::runtime::obj::persistent_hash_map::empty(),
    ns,
    name);
}

jank::runtime::obj::integer_ref _jank_int(jtl::i64 const i)
{
  return jank::runtime::make_box<jank::runtime::obj::integer>(i);
}

jank::runtime::obj::small_integer_ref _jank_small_int(jtl::i64 const i)
{
  return i;
}

jank::runtime::obj::real_ref _jank_real(jtl::f64 const r)
{
  return jank::runtime::make_box<jank::runtime::obj::real>(r);
}

jank::runtime::obj::symbol_ref
_jank_symbol(jank::runtime::object_ref const meta, char const * const ns, char const * const name)
{
  return jank::runtime::make_box<jank::runtime::obj::symbol>(meta, ns, name);
}

jank::runtime::obj::persistent_string_ref _jank_string(char const * const s)
{
  return jank::runtime::make_box<jank::runtime::obj::persistent_string>(s);
}

jank::runtime::obj::jit_function_ref _jank_fn(jank::runtime::callable_arity_flags const flags)
{
  return jank::runtime::make_box<jank::runtime::obj::jit_function>(flags);
}

jank::runtime::obj::jit_closure_ref
_jank_closure(jank::runtime::callable_arity_flags const flags, void * const ctx)
{
  return jank::runtime::make_box<jank::runtime::obj::jit_closure>(flags, ctx);
}

jank::runtime::object_ref _jank_read(char const *edn)
{
  return jank::runtime::__rt_ctx->forcefully_read_string(edn);
}
