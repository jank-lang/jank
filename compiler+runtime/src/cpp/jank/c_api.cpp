#include <cstdarg>
#include <cstdint>

#include <utility>

#include <jank/c_api.h>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core.hpp>
#include <jank/profile/time.hpp>
#include <jank/util/scope_exit.hpp>

using namespace jank;
using namespace jank::runtime;

template <typename Is>
struct make_closure_arity;

template <size_t I>
struct make_closure_arity_arg
{
  using type = object *;
};

template <size_t... Is>
struct make_closure_arity<std::index_sequence<Is...>>
{
  using type = object *(*)(void *, typename make_closure_arity_arg<Is>::type...);
};

template <>
struct make_closure_arity<std::index_sequence<>>
{
  using type = object *(*)(void *);
};

template <size_t N>
using closure_arity = typename make_closure_arity<std::make_index_sequence<N>>::type;

template <typename Is>
struct make_function_arity;

template <size_t I>
struct make_function_arity_arg
{
  using type = object *;
};

template <size_t... Is>
struct make_function_arity<std::index_sequence<Is...>>
{
  using type = object *(*)(typename make_function_arity_arg<Is>::type...);
};

template <>
struct make_function_arity<std::index_sequence<>>
{
  using type = object *(*)();
};

template <size_t N>
using function_arity = typename make_function_arity<std::make_index_sequence<N>>::type;

extern "C"
{
  jank_object_ptr jank_eval(jank_object_ptr const s)
  {
    auto const s_obj(try_object<obj::persistent_string>(reinterpret_cast<object *>(s)));
    return __rt_ctx->eval_string(s_obj->data);
  }

  jank_object_ptr jank_read_string(jank_object_ptr const s)
  {
    auto const s_obj(try_object<obj::persistent_string>(reinterpret_cast<object *>(s)));
    return __rt_ctx->read_string(s_obj->data);
  }

  jank_object_ptr jank_read_string_c(char const * const s)
  {
    return __rt_ctx->read_string(s);
  }

  void jank_ns_set_symbol_counter(char const * const ns, uint64_t const count)
  {
    auto const ns_obj(__rt_ctx->intern_ns(ns));
    ns_obj->symbol_counter.store(count);
  }

  jank_object_ptr jank_var_intern(jank_object_ptr const ns, jank_object_ptr const name)
  {
    auto const ns_obj(try_object<obj::persistent_string>(reinterpret_cast<object *>(ns)));
    __rt_ctx->intern_ns(ns_obj->data);

    auto const name_obj(try_object<obj::persistent_string>(reinterpret_cast<object *>(name)));
    return erase(__rt_ctx->intern_var(ns_obj->data, name_obj->data).expect_ok());
  }

  jank_object_ptr jank_var_bind_root(jank_object_ptr const var, jank_object_ptr const val)
  {
    auto const var_obj(try_object<runtime::var>(reinterpret_cast<object *>(var)));
    auto const val_obj(reinterpret_cast<object *>(val));
    return erase(var_obj->bind_root(val_obj));
  }

  jank_object_ptr jank_var_set_dynamic(jank_object_ptr const var, jank_object_ptr const dynamic)
  {
    auto const var_obj(try_object<runtime::var>(reinterpret_cast<object *>(var)));
    auto const dynamic_obj(reinterpret_cast<object *>(dynamic));
    return erase(var_obj->set_dynamic(truthy(dynamic_obj)));
  }

  jank_object_ptr jank_keyword_intern(jank_object_ptr const ns, jank_object_ptr const name)
  {
    auto const ns_obj(reinterpret_cast<object *>(ns));
    auto const name_obj(reinterpret_cast<object *>(name));
    return erase(__rt_ctx->intern_keyword(to_string(ns_obj), to_string(name_obj)).expect_ok());
  }

  jank_object_ptr jank_deref(jank_object_ptr const o)
  {
    auto const o_obj(reinterpret_cast<object *>(o));
    return deref(o_obj);
  }

  jank_object_ptr jank_call0(jank_object_ptr const f)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    return dynamic_call(f_obj);
  }

  jank_object_ptr jank_call1(jank_object_ptr const f, jank_object_ptr const a1)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    auto const a1_obj(reinterpret_cast<object *>(a1));
    return dynamic_call(f_obj, a1_obj);
  }

  jank_object_ptr
  jank_call2(jank_object_ptr const f, jank_object_ptr const a1, jank_object_ptr const a2)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    auto const a1_obj(reinterpret_cast<object *>(a1));
    auto const a2_obj(reinterpret_cast<object *>(a2));
    return dynamic_call(f_obj, a1_obj, a2_obj);
  }

  jank_object_ptr jank_call3(jank_object_ptr const f,
                             jank_object_ptr const a1,
                             jank_object_ptr const a2,
                             jank_object_ptr const a3)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    auto const a1_obj(reinterpret_cast<object *>(a1));
    auto const a2_obj(reinterpret_cast<object *>(a2));
    auto const a3_obj(reinterpret_cast<object *>(a3));
    return dynamic_call(f_obj, a1_obj, a2_obj, a3_obj);
  }

  jank_object_ptr jank_call4(jank_object_ptr const f,
                             jank_object_ptr const a1,
                             jank_object_ptr const a2,
                             jank_object_ptr const a3,
                             jank_object_ptr const a4)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    auto const a1_obj(reinterpret_cast<object *>(a1));
    auto const a2_obj(reinterpret_cast<object *>(a2));
    auto const a3_obj(reinterpret_cast<object *>(a3));
    auto const a4_obj(reinterpret_cast<object *>(a4));
    return dynamic_call(f_obj, a1_obj, a2_obj, a3_obj, a4_obj);
  }

  jank_object_ptr jank_call5(jank_object_ptr const f,
                             jank_object_ptr const a1,
                             jank_object_ptr const a2,
                             jank_object_ptr const a3,
                             jank_object_ptr const a4,
                             jank_object_ptr const a5)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    auto const a1_obj(reinterpret_cast<object *>(a1));
    auto const a2_obj(reinterpret_cast<object *>(a2));
    auto const a3_obj(reinterpret_cast<object *>(a3));
    auto const a4_obj(reinterpret_cast<object *>(a4));
    auto const a5_obj(reinterpret_cast<object *>(a5));
    return dynamic_call(f_obj, a1_obj, a2_obj, a3_obj, a4_obj, a5_obj);
  }

  jank_object_ptr jank_call6(jank_object_ptr const f,
                             jank_object_ptr const a1,
                             jank_object_ptr const a2,
                             jank_object_ptr const a3,
                             jank_object_ptr const a4,
                             jank_object_ptr const a5,
                             jank_object_ptr const a6)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    auto const a1_obj(reinterpret_cast<object *>(a1));
    auto const a2_obj(reinterpret_cast<object *>(a2));
    auto const a3_obj(reinterpret_cast<object *>(a3));
    auto const a4_obj(reinterpret_cast<object *>(a4));
    auto const a5_obj(reinterpret_cast<object *>(a5));
    auto const a6_obj(reinterpret_cast<object *>(a6));
    return dynamic_call(f_obj, a1_obj, a2_obj, a3_obj, a4_obj, a5_obj, a6_obj);
  }

  jank_object_ptr jank_call7(jank_object_ptr const f,
                             jank_object_ptr const a1,
                             jank_object_ptr const a2,
                             jank_object_ptr const a3,
                             jank_object_ptr const a4,
                             jank_object_ptr const a5,
                             jank_object_ptr const a6,
                             jank_object_ptr const a7)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    auto const a1_obj(reinterpret_cast<object *>(a1));
    auto const a2_obj(reinterpret_cast<object *>(a2));
    auto const a3_obj(reinterpret_cast<object *>(a3));
    auto const a4_obj(reinterpret_cast<object *>(a4));
    auto const a5_obj(reinterpret_cast<object *>(a5));
    auto const a6_obj(reinterpret_cast<object *>(a6));
    auto const a7_obj(reinterpret_cast<object *>(a7));
    return dynamic_call(f_obj, a1_obj, a2_obj, a3_obj, a4_obj, a5_obj, a6_obj, a7_obj);
  }

  jank_object_ptr jank_call8(jank_object_ptr const f,
                             jank_object_ptr const a1,
                             jank_object_ptr const a2,
                             jank_object_ptr const a3,
                             jank_object_ptr const a4,
                             jank_object_ptr const a5,
                             jank_object_ptr const a6,
                             jank_object_ptr const a7,
                             jank_object_ptr const a8)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    auto const a1_obj(reinterpret_cast<object *>(a1));
    auto const a2_obj(reinterpret_cast<object *>(a2));
    auto const a3_obj(reinterpret_cast<object *>(a3));
    auto const a4_obj(reinterpret_cast<object *>(a4));
    auto const a5_obj(reinterpret_cast<object *>(a5));
    auto const a6_obj(reinterpret_cast<object *>(a6));
    auto const a7_obj(reinterpret_cast<object *>(a7));
    auto const a8_obj(reinterpret_cast<object *>(a8));
    return dynamic_call(f_obj, a1_obj, a2_obj, a3_obj, a4_obj, a5_obj, a6_obj, a7_obj, a8_obj);
  }

  jank_object_ptr jank_call9(jank_object_ptr const f,
                             jank_object_ptr const a1,
                             jank_object_ptr const a2,
                             jank_object_ptr const a3,
                             jank_object_ptr const a4,
                             jank_object_ptr const a5,
                             jank_object_ptr const a6,
                             jank_object_ptr const a7,
                             jank_object_ptr const a8,
                             jank_object_ptr const a9)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    auto const a1_obj(reinterpret_cast<object *>(a1));
    auto const a2_obj(reinterpret_cast<object *>(a2));
    auto const a3_obj(reinterpret_cast<object *>(a3));
    auto const a4_obj(reinterpret_cast<object *>(a4));
    auto const a5_obj(reinterpret_cast<object *>(a5));
    auto const a6_obj(reinterpret_cast<object *>(a6));
    auto const a7_obj(reinterpret_cast<object *>(a7));
    auto const a8_obj(reinterpret_cast<object *>(a8));
    auto const a9_obj(reinterpret_cast<object *>(a9));
    return dynamic_call(f_obj,
                        a1_obj,
                        a2_obj,
                        a3_obj,
                        a4_obj,
                        a5_obj,
                        a6_obj,
                        a7_obj,
                        a8_obj,
                        a9_obj);
  }

  jank_object_ptr jank_call10(jank_object_ptr const f,
                              jank_object_ptr const a1,
                              jank_object_ptr const a2,
                              jank_object_ptr const a3,
                              jank_object_ptr const a4,
                              jank_object_ptr const a5,
                              jank_object_ptr const a6,
                              jank_object_ptr const a7,
                              jank_object_ptr const a8,
                              jank_object_ptr const a9,
                              jank_object_ptr const a10)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    auto const a1_obj(reinterpret_cast<object *>(a1));
    auto const a2_obj(reinterpret_cast<object *>(a2));
    auto const a3_obj(reinterpret_cast<object *>(a3));
    auto const a4_obj(reinterpret_cast<object *>(a4));
    auto const a5_obj(reinterpret_cast<object *>(a5));
    auto const a6_obj(reinterpret_cast<object *>(a6));
    auto const a7_obj(reinterpret_cast<object *>(a7));
    auto const a8_obj(reinterpret_cast<object *>(a8));
    auto const a9_obj(reinterpret_cast<object *>(a9));
    auto const a10_obj(reinterpret_cast<object *>(a10));
    return dynamic_call(f_obj,
                        a1_obj,
                        a2_obj,
                        a3_obj,
                        a4_obj,
                        a5_obj,
                        a6_obj,
                        a7_obj,
                        a8_obj,
                        a9_obj,
                        a10_obj);
  }

  jank_object_ptr jank_call11(jank_object_ptr const f,
                              jank_object_ptr const a1,
                              jank_object_ptr const a2,
                              jank_object_ptr const a3,
                              jank_object_ptr const a4,
                              jank_object_ptr const a5,
                              jank_object_ptr const a6,
                              jank_object_ptr const a7,
                              jank_object_ptr const a8,
                              jank_object_ptr const a9,
                              jank_object_ptr const a10,
                              jank_object_ptr const rest)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    auto const a1_obj(reinterpret_cast<object *>(a1));
    auto const a2_obj(reinterpret_cast<object *>(a2));
    auto const a3_obj(reinterpret_cast<object *>(a3));
    auto const a4_obj(reinterpret_cast<object *>(a4));
    auto const a5_obj(reinterpret_cast<object *>(a5));
    auto const a6_obj(reinterpret_cast<object *>(a6));
    auto const a7_obj(reinterpret_cast<object *>(a7));
    auto const a8_obj(reinterpret_cast<object *>(a8));
    auto const a9_obj(reinterpret_cast<object *>(a9));
    auto const a10_obj(reinterpret_cast<object *>(a10));
    auto const rest_obj(reinterpret_cast<object *>(rest));
    return dynamic_call(f_obj,
                        a1_obj,
                        a2_obj,
                        a3_obj,
                        a4_obj,
                        a5_obj,
                        a6_obj,
                        a7_obj,
                        a8_obj,
                        a9_obj,
                        a10_obj,
                        try_object<obj::persistent_list>(rest_obj));
  }

  jank_object_ptr jank_nil()
  {
    return erase(obj::nil::nil_const());
  }

  jank_object_ptr jank_true()
  {
    return erase(obj::boolean::true_const());
  }

  jank_object_ptr jank_false()
  {
    return erase(obj::boolean::false_const());
  }

  jank_object_ptr jank_integer_create(jank_native_integer const i)
  {
    return erase(make_box(i));
  }

  jank_object_ptr jank_big_integer_create(char const *s)
  {
    assert(s);
    return erase(make_box<runtime::obj::big_integer>(jank::native_persistent_string_view{ s }));
  }

  jank_object_ptr jank_real_create(jank_native_real const r)
  {
    return erase(make_box(r));
  }

  jank_object_ptr
  jank_ratio_create(jank_object_ptr const numerator, jank_object_ptr const denominator)
  {
    return erase(make_box(runtime::obj::ratio_data(reinterpret_cast<object *>(numerator),
                                                   reinterpret_cast<object *>(denominator))));
  }

  jank_object_ptr jank_string_create(char const *s)
  {
    assert(s);
    return erase(make_box(s));
  }

  jank_object_ptr jank_symbol_create(jank_object_ptr const ns, jank_object_ptr const name)
  {
    auto const ns_obj(reinterpret_cast<object *>(ns));
    auto const name_obj(reinterpret_cast<object *>(name));
    return erase(make_box<obj::symbol>(ns_obj, name_obj));
  }

  jank_object_ptr jank_character_create(char const *s)
  {
    assert(s);
    return erase(make_box<obj::character>(read::parse::get_char_from_literal(s).unwrap()));
  }

  jank_object_ptr jank_list_create(uint64_t const size, ...)
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    va_list args{};
    va_start(args, size);

    native_vector<object_ptr> v;

    for(uint64_t i{}; i < size; ++i)
    {
      /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
      v.emplace_back(reinterpret_cast<object *>(va_arg(args, jank_object_ptr)));
    }

    va_end(args);

    runtime::detail::native_persistent_list const npl{ v.rbegin(), v.rend() };
    return erase(make_box<obj::persistent_list>(std::move(npl)));
  }

  jank_object_ptr jank_vector_create(uint64_t const size, ...)
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    va_list args{};
    va_start(args, size);

    obj::transient_vector trans;

    for(uint64_t i{}; i < size; ++i)
    {
      /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
      trans.conj_in_place(reinterpret_cast<object *>(va_arg(args, jank_object_ptr)));
    }

    va_end(args);
    return erase(trans.to_persistent());
  }

  /* TODO: Meta for maps, vectors, sets, symbols, and fns. */
  jank_object_ptr jank_map_create(uint64_t const pairs, ...)
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    va_list args{};
    va_start(args, pairs);

    /* TODO: Could optimize to build an array map, if it's small enough. */
    obj::transient_hash_map trans;

    for(uint64_t i{}; i < pairs; ++i)
    {
      /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
      trans.assoc_in_place(reinterpret_cast<object *>(va_arg(args, jank_object_ptr)),
                           /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
                           reinterpret_cast<object *>(va_arg(args, jank_object_ptr)));
    }

    va_end(args);
    return erase(trans.to_persistent());
  }

  jank_object_ptr jank_set_create(uint64_t const size, ...)
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    va_list args{};
    va_start(args, size);

    obj::transient_hash_set trans;

    for(uint64_t i{}; i < size; ++i)
    {
      /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
      trans.conj_in_place(reinterpret_cast<object *>(va_arg(args, jank_object_ptr)));
    }

    va_end(args);
    return erase(trans.to_persistent());
  }

  jank_arity_flags jank_function_build_arity_flags(uint8_t const highest_fixed_arity,
                                                   jank_native_bool const is_variadic,
                                                   jank_native_bool const is_variadic_ambiguous)
  {
    return behavior::callable::build_arity_flags(highest_fixed_arity,
                                                 is_variadic,
                                                 is_variadic_ambiguous);
  }

  jank_object_ptr jank_function_create(jank_arity_flags const arity_flags)
  {
    return erase(make_box<obj::jit_function>(arity_flags));
  }

  void jank_function_set_arity0(jank_object_ptr const fn, jank_object_ptr (* const f)())
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_function>(fn_obj)->arity_0 = reinterpret_cast<function_arity<0>>(f);
#pragma clang diagnostic pop
  }

  void
  jank_function_set_arity1(jank_object_ptr const fn, jank_object_ptr (* const f)(jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_function>(fn_obj)->arity_1 = reinterpret_cast<function_arity<1>>(f);
#pragma clang diagnostic pop
  }

  void jank_function_set_arity2(jank_object_ptr const fn,
                                jank_object_ptr (* const f)(jank_object_ptr, jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_function>(fn_obj)->arity_2 = reinterpret_cast<function_arity<2>>(f);
#pragma clang diagnostic pop
  }

  void jank_function_set_arity3(jank_object_ptr const fn,
                                jank_object_ptr (* const f)(jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_function>(fn_obj)->arity_3 = reinterpret_cast<function_arity<3>>(f);
#pragma clang diagnostic pop
  }

  void jank_function_set_arity4(
    jank_object_ptr fn,
    jank_object_ptr (* const f)(jank_object_ptr, jank_object_ptr, jank_object_ptr, jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_function>(fn_obj)->arity_4 = reinterpret_cast<function_arity<4>>(f);
#pragma clang diagnostic pop
  }

  void jank_function_set_arity5(jank_object_ptr fn,
                                jank_object_ptr (* const f)(jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_function>(fn_obj)->arity_5 = reinterpret_cast<function_arity<5>>(f);
#pragma clang diagnostic pop
  }

  void jank_function_set_arity6(jank_object_ptr fn,
                                jank_object_ptr (* const f)(jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_function>(fn_obj)->arity_6 = reinterpret_cast<function_arity<6>>(f);
#pragma clang diagnostic pop
  }

  void jank_function_set_arity7(jank_object_ptr fn,
                                jank_object_ptr (* const f)(jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_function>(fn_obj)->arity_7 = reinterpret_cast<function_arity<7>>(f);
#pragma clang diagnostic pop
  }

  void jank_function_set_arity8(jank_object_ptr fn,
                                jank_object_ptr (* const f)(jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_function>(fn_obj)->arity_8 = reinterpret_cast<function_arity<8>>(f);
#pragma clang diagnostic pop
  }

  void jank_function_set_arity9(jank_object_ptr fn,
                                jank_object_ptr (* const f)(jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_function>(fn_obj)->arity_9 = reinterpret_cast<function_arity<9>>(f);
#pragma clang diagnostic pop
  }

  void jank_function_set_arity10(jank_object_ptr fn,
                                 jank_object_ptr (* const f)(jank_object_ptr,
                                                             jank_object_ptr,
                                                             jank_object_ptr,
                                                             jank_object_ptr,
                                                             jank_object_ptr,
                                                             jank_object_ptr,
                                                             jank_object_ptr,
                                                             jank_object_ptr,
                                                             jank_object_ptr,
                                                             jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_function>(fn_obj)->arity_10 = reinterpret_cast<function_arity<10>>(f);
#pragma clang diagnostic pop
  }

  jank_object_ptr jank_closure_create(jank_arity_flags const arity_flags, void * const context)
  {
    return erase(make_box<obj::jit_closure>(arity_flags, context));
  }

  void jank_closure_set_arity0(jank_object_ptr const fn, jank_object_ptr (* const f)())
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_closure>(fn_obj)->arity_0 = reinterpret_cast<closure_arity<0>>(f);
#pragma clang diagnostic pop
  }

  void
  jank_closure_set_arity1(jank_object_ptr const fn, jank_object_ptr (* const f)(jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_closure>(fn_obj)->arity_1 = reinterpret_cast<closure_arity<1>>(f);
#pragma clang diagnostic pop
  }

  void jank_closure_set_arity2(jank_object_ptr const fn,
                               jank_object_ptr (* const f)(jank_object_ptr, jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_closure>(fn_obj)->arity_2 = reinterpret_cast<closure_arity<2>>(f);
#pragma clang diagnostic pop
  }

  void jank_closure_set_arity3(jank_object_ptr const fn,
                               jank_object_ptr (* const f)(jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_closure>(fn_obj)->arity_3 = reinterpret_cast<closure_arity<3>>(f);
#pragma clang diagnostic pop
  }

  void jank_closure_set_arity4(
    jank_object_ptr fn,
    jank_object_ptr (* const f)(jank_object_ptr, jank_object_ptr, jank_object_ptr, jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_closure>(fn_obj)->arity_4 = reinterpret_cast<closure_arity<4>>(f);
#pragma clang diagnostic pop
  }

  void jank_closure_set_arity5(jank_object_ptr fn,
                               jank_object_ptr (* const f)(jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_closure>(fn_obj)->arity_5 = reinterpret_cast<closure_arity<5>>(f);
#pragma clang diagnostic pop
  }

  void jank_closure_set_arity6(jank_object_ptr fn,
                               jank_object_ptr (* const f)(jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_closure>(fn_obj)->arity_6 = reinterpret_cast<closure_arity<6>>(f);
#pragma clang diagnostic pop
  }

  void jank_closure_set_arity7(jank_object_ptr fn,
                               jank_object_ptr (* const f)(jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_closure>(fn_obj)->arity_7 = reinterpret_cast<closure_arity<7>>(f);
#pragma clang diagnostic pop
  }

  void jank_closure_set_arity8(jank_object_ptr fn,
                               jank_object_ptr (* const f)(jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_closure>(fn_obj)->arity_8 = reinterpret_cast<closure_arity<8>>(f);
#pragma clang diagnostic pop
  }

  void jank_closure_set_arity9(jank_object_ptr fn,
                               jank_object_ptr (* const f)(jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr,
                                                           jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_closure>(fn_obj)->arity_9 = reinterpret_cast<closure_arity<9>>(f);
#pragma clang diagnostic pop
  }

  void jank_closure_set_arity10(jank_object_ptr fn,
                                jank_object_ptr (* const f)(jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr,
                                                            jank_object_ptr))
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
    auto const fn_obj(reinterpret_cast<object *>(fn));
    try_object<obj::jit_closure>(fn_obj)->arity_10 = reinterpret_cast<closure_arity<10>>(f);
#pragma clang diagnostic pop
  }

  jank_native_bool jank_truthy(jank_object_ptr const o)
  {
    auto const o_obj(reinterpret_cast<object *>(o));
    return static_cast<jank_native_bool>(truthy(o_obj));
  }

  jank_native_bool jank_equal(jank_object_ptr const l, jank_object_ptr const r)
  {
    auto const l_obj(reinterpret_cast<object *>(l));
    auto const r_obj(reinterpret_cast<object *>(r));
    return static_cast<jank_native_bool>(equal(l_obj, r_obj));
  }

  jank_native_hash jank_to_hash(jank_object_ptr const o)
  {
    auto const o_obj(reinterpret_cast<object *>(o));
    return to_hash(o_obj);
  }

  static native_integer to_integer_or_hash(object const *o)
  {
    if(o->type == object_type::integer)
    {
      return expect_object<obj::integer>(o)->data;
    }

    return to_hash(o);
  }

  jank_native_integer jank_to_integer(jank_object_ptr const o)
  {
    auto const o_obj(reinterpret_cast<object *>(o));
    return to_integer_or_hash(o_obj);
  }

  jank_native_integer jank_shift_mask_case_integer(jank_object_ptr const o,
                                                   jank_native_integer const shift,
                                                   jank_native_integer const mask)
  {
    auto const o_obj(reinterpret_cast<object *>(o));
    auto integer{ to_integer_or_hash(o_obj) };
    if(mask != 0)
    {
      if(o_obj->type == object_type::integer)
      {
        /* We don't hash the integer if it's an int32 value. This is to be consistent with how keys are hashed in jank's
         * case macro. */
        integer = (integer >= std::numeric_limits<int32_t>::min()
                   && integer <= std::numeric_limits<int32_t>::max())
          ? integer
          : hash::integer(integer);
      }
      integer = (integer >> shift) & mask;
    }
    return integer;
  }

  void jank_set_meta(jank_object_ptr const o, jank_object_ptr const meta)
  {
    auto const o_obj(reinterpret_cast<object *>(o));
    auto const meta_obj(reinterpret_cast<object *>(meta));
    runtime::visit_object(
      [&](auto const typed_o) {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::metadatable<T>)
        {
          typed_o->meta = behavior::detail::validate_meta(meta_obj);
        }
      },
      o_obj);
  }

  void jank_throw(jank_object_ptr const o)
  {
    throw runtime::object_ptr{ reinterpret_cast<object *>(o) };
  }

  jank_object_ptr jank_try(jank_object_ptr const try_fn,
                           jank_object_ptr const catch_fn,
                           jank_object_ptr const finally_fn)
  {
    util::scope_exit const finally{ [=]() {
      auto const finally_fn_obj(reinterpret_cast<object *>(finally_fn));
      if(finally_fn_obj != obj::nil::nil_const())
      {
        dynamic_call(finally_fn_obj);
      }
    } };

    auto const try_fn_obj(reinterpret_cast<object *>(try_fn));
    auto const catch_fn_obj(reinterpret_cast<object *>(catch_fn));
    if(catch_fn_obj == obj::nil::nil_const())
    {
      return dynamic_call(try_fn_obj);
    }
    try
    {
      return dynamic_call(try_fn_obj);
    }
    catch(object_ptr const e)
    {
      return dynamic_call(catch_fn_obj, e);
    }
  }

  void jank_profile_enter(char const * const label)
  {
    profile::enter(label);
  }

  void jank_profile_exit(char const * const label)
  {
    profile::exit(label);
  }

  void jank_profile_report(char const * const label)
  {
    profile::report(label);
  }
}
