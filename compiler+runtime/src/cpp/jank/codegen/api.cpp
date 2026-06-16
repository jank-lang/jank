#include <cstdarg>

#include <jank/codegen/api.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/transient_hash_map.hpp>
#include <jank/runtime/obj/transient_array_map.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/transient_vector.hpp>
#include <jank/runtime/obj/transient_hash_set.hpp>

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
  return jank::runtime::make_box<jank::runtime::obj::symbol>(ns, name);
}

jank::runtime::obj::symbol_ref
_jank_symbol(char const * const meta, char const * const ns, char const * const name)
{
  return jank::runtime::make_box<jank::runtime::obj::symbol>(meta, ns, name);
}

jank::runtime::obj::integer_ref _jank_int(jtl::i64 const i)
{
  return jank::runtime::make_box<jank::runtime::obj::integer>(i);
}

jank::runtime::obj::small_integer_ref _jank_small_int(jtl::i32 const i)
{
  return i;
}

jank::runtime::obj::real_ref _jank_real(jtl::f64 const r)
{
  return jank::runtime::make_box<jank::runtime::obj::real>(r);
}

jank::runtime::obj::small_real_ref _jank_small_real(jtl::f64 const r)
{
  return r;
}

jank::runtime::obj::persistent_string_ref _jank_string()
{
  return jank::runtime::obj::persistent_string::empty();
}

jank::runtime::obj::persistent_string_ref _jank_string(char const * const s)
{
  return jank::runtime::make_box<jank::runtime::obj::persistent_string>(s);
}

static jank::runtime::obj::persistent_list_ref
_jank_list_impl(char const * const meta, jank::u64 const elems, va_list args)
{
  jank::native_vector<jank::runtime::object_ref> trans;
  trans.reserve(elems);

  for(jank::u64 i{}; i < elems; ++i)
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    trans.emplace_back(va_arg(args, jank::runtime::object *));
  }

  jank::runtime::detail::native_persistent_list list{ trans.rbegin(), trans.rend() };
  if(meta)
  {
    return jank::runtime::make_box<jank::runtime::obj::persistent_list>(
      jank::runtime::lazy_meta{ meta },
      jtl::move(list));
  }
  return jank::runtime::make_box<jank::runtime::obj::persistent_list>(jtl::move(list));
}

/* NOLINTNEXTLINE(modernize-avoid-variadic-functions) */
jank::runtime::obj::persistent_list_ref _jank_list(jank::u64 const elems, ...)
{
  /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
  va_list args;
  va_start(args, elems);
  auto const ret{ _jank_list_impl(nullptr, elems, args) };
  va_end(args);
  return ret;
}

jank::runtime::obj::persistent_list_ref
/* NOLINTNEXTLINE(modernize-avoid-variadic-functions) */
_jank_list(char const * const meta, jank::u64 const elems, ...)
{
  /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
  va_list args;
  va_start(args, elems);
  auto const ret{ _jank_list_impl(meta, elems, args) };
  va_end(args);
  return ret;
}

static jank::runtime::obj::persistent_vector_ref
_jank_vec_impl(char const * const meta, jank::u64 const elems, va_list args)
{
  jank::runtime::obj::transient_vector trans;

  for(jank::u64 i{}; i < elems; ++i)
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    trans.conj_in_place(va_arg(args, jank::runtime::object *));
  }

  if(meta)
  {
    return trans.to_persistent(meta);
  }
  return trans.to_persistent();
}

/* NOLINTNEXTLINE(modernize-avoid-variadic-functions) */
jank::runtime::obj::persistent_vector_ref _jank_vec(jank::u64 const elems, ...)
{
  /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
  va_list args;
  va_start(args, elems);
  auto const ret{ _jank_vec_impl(nullptr, elems, args) };
  va_end(args);
  return ret;
}

jank::runtime::obj::persistent_vector_ref
/* NOLINTNEXTLINE(modernize-avoid-variadic-functions) */
_jank_vec(char const * const meta, jank::u64 const elems, ...)
{
  /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
  va_list args;
  va_start(args, elems);
  auto const ret{ _jank_vec_impl(meta, elems, args) };
  va_end(args);
  return ret;
}

static jank::runtime::obj::persistent_hash_set_ref
_jank_hset_impl(char const * const meta, jank::u64 const elems, va_list args)
{
  jank::runtime::obj::transient_hash_set trans;

  for(jank::u64 i{}; i < elems; ++i)
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    trans.conj_in_place(va_arg(args, jank::runtime::object *));
  }

  if(meta)
  {
    return trans.to_persistent(meta);
  }
  return trans.to_persistent();
}

/* NOLINTNEXTLINE(modernize-avoid-variadic-functions) */
jank::runtime::obj::persistent_hash_set_ref _jank_hset(jank::u64 const elems, ...)
{
  /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
  va_list args;
  va_start(args, elems);
  auto const ret{ _jank_hset_impl(nullptr, elems, args) };
  va_end(args);
  return ret;
}

jank::runtime::obj::persistent_hash_set_ref
/* NOLINTNEXTLINE(modernize-avoid-variadic-functions) */
_jank_hset(char const * const meta, jank::u64 const elems, ...)
{
  /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
  va_list args;
  va_start(args, elems);
  auto const ret{ _jank_hset_impl(meta, elems, args) };
  va_end(args);
  return ret;
}

static jank::runtime::obj::persistent_array_map_ref
_jank_amap_impl(char const * const meta, jank::u64 const pairs, va_list args)
{
  jank::runtime::obj::transient_array_map trans;
  trans.data.reserve(pairs);

  for(jank::u64 i{}; i < pairs; ++i)
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    trans.assoc_in_place(va_arg(args, jank::runtime::object *),
                         /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
                         va_arg(args, jank::runtime::object *));
  }

  if(meta)
  {
    return trans.to_persistent(meta);
  }
  return trans.to_persistent();
}

/* NOLINTNEXTLINE(modernize-avoid-variadic-functions) */
jank::runtime::obj::persistent_array_map_ref _jank_amap(jank::u64 const pairs, ...)
{
  /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
  va_list args;
  va_start(args, pairs);
  auto const ret{ _jank_amap_impl(nullptr, pairs, args) };
  va_end(args);
  return ret;
}

jank::runtime::obj::persistent_array_map_ref
/* NOLINTNEXTLINE(modernize-avoid-variadic-functions) */
_jank_amap(char const * const meta, jank::u64 const pairs, ...)
{
  /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
  va_list args;
  va_start(args, pairs);
  auto const ret{ _jank_amap_impl(meta, pairs, args) };
  va_end(args);
  return ret;
}

static jank::runtime::obj::persistent_hash_map_ref
_jank_hmap_impl(char const * const meta, jank::u64 const pairs, va_list args)
{
  jank::runtime::obj::transient_hash_map trans;

  for(jank::u64 i{}; i < pairs; ++i)
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    trans.assoc_in_place(va_arg(args, jank::runtime::object *),
                         /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
                         va_arg(args, jank::runtime::object *));
  }

  if(meta)
  {
    return trans.to_persistent(meta);
  }
  return trans.to_persistent();
}

/* NOLINTNEXTLINE(modernize-avoid-variadic-functions) */
jank::runtime::obj::persistent_hash_map_ref _jank_hmap(jank::u64 const pairs, ...)
{
  /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
  va_list args;
  va_start(args, pairs);
  auto const ret{ _jank_hmap_impl(nullptr, pairs, args) };
  va_end(args);
  return ret;
}

jank::runtime::obj::persistent_hash_map_ref
/* NOLINTNEXTLINE(modernize-avoid-variadic-functions) */
_jank_hmap(char const * const meta, jank::u64 const pairs, ...)
{
  /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
  va_list args;
  va_start(args, pairs);
  auto const ret{ _jank_hmap_impl(meta, pairs, args) };
  va_end(args);
  return ret;
}

jank::runtime::obj::jit_function_ref _jank_fn(jank::runtime::callable_arity_flags const flags)
{
  return jank::runtime::make_box<jank::runtime::obj::jit_function>(flags);
}

jank::runtime::obj::jit_variadic_function_ref
_jank_vfn(jank::runtime::callable_arity_flags const flags)
{
  return jank::runtime::make_box<jank::runtime::obj::jit_variadic_function>(flags);
}

jank::runtime::obj::jit_closure_ref
_jank_closure(jank::runtime::callable_arity_flags const flags, void * const ctx)
{
  return jank::runtime::make_box<jank::runtime::obj::jit_closure>(flags, ctx);
}

jank::runtime::obj::jit_variadic_closure_ref
_jank_vclosure(jank::runtime::callable_arity_flags const flags, void * const ctx)
{
  return jank::runtime::make_box<jank::runtime::obj::jit_variadic_closure>(flags, ctx);
}
