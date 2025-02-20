#pragma once

/* NOLINTNEXTLINE(modernize-deprecated-headers) */
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /* NOLINTNEXTLINE(modernize-use-using) */
  typedef void *jank_object_ptr;
  /* NOLINTNEXTLINE(modernize-use-using) */
  typedef long long jank_native_integer;
  /* NOLINTNEXTLINE(modernize-use-using) */
  typedef double jank_native_real;
  /* NOLINTNEXTLINE(modernize-use-using) */
  typedef char jank_native_bool;
  /* NOLINTNEXTLINE(modernize-use-using) */
  typedef uint32_t jank_native_hash;
  /* NOLINTNEXTLINE(modernize-use-using) */
  typedef uint8_t jank_arity_flags;

  jank_object_ptr jank_eval(jank_object_ptr s);
  jank_object_ptr jank_read_string(jank_object_ptr s);

  void jank_ns_set_symbol_counter(char const * const ns, uint64_t const count);

  jank_object_ptr jank_var_intern(jank_object_ptr ns, jank_object_ptr name);
  jank_object_ptr jank_var_bind_root(jank_object_ptr var, jank_object_ptr val);
  jank_object_ptr jank_var_set_dynamic(jank_object_ptr var, jank_object_ptr dynamic);

  jank_object_ptr jank_keyword_intern(jank_object_ptr ns, jank_object_ptr name);

  jank_object_ptr jank_deref(jank_object_ptr o);
  jank_object_ptr jank_blocking_deref(jank_object_ptr f, jank_object_ptr millis, jank_object_ptr timeout_value);

  jank_object_ptr jank_call0(jank_object_ptr f);
  jank_object_ptr jank_call1(jank_object_ptr f, jank_object_ptr a1);
  jank_object_ptr jank_call2(jank_object_ptr f, jank_object_ptr a1, jank_object_ptr a2);
  jank_object_ptr
  jank_call3(jank_object_ptr f, jank_object_ptr a1, jank_object_ptr a2, jank_object_ptr a3);
  jank_object_ptr jank_call4(jank_object_ptr f,
                             jank_object_ptr a1,
                             jank_object_ptr a2,
                             jank_object_ptr a3,
                             jank_object_ptr a4);
  jank_object_ptr jank_call5(jank_object_ptr f,
                             jank_object_ptr a1,
                             jank_object_ptr a2,
                             jank_object_ptr a3,
                             jank_object_ptr a4,
                             jank_object_ptr a5);
  jank_object_ptr jank_call6(jank_object_ptr f,
                             jank_object_ptr a1,
                             jank_object_ptr a2,
                             jank_object_ptr a3,
                             jank_object_ptr a4,
                             jank_object_ptr a5,
                             jank_object_ptr a6);
  jank_object_ptr jank_call7(jank_object_ptr f,
                             jank_object_ptr a1,
                             jank_object_ptr a2,
                             jank_object_ptr a3,
                             jank_object_ptr a4,
                             jank_object_ptr a5,
                             jank_object_ptr a6,
                             jank_object_ptr a7);
  jank_object_ptr jank_call8(jank_object_ptr f,
                             jank_object_ptr a1,
                             jank_object_ptr a2,
                             jank_object_ptr a3,
                             jank_object_ptr a4,
                             jank_object_ptr a5,
                             jank_object_ptr a6,
                             jank_object_ptr a7,
                             jank_object_ptr a8);
  jank_object_ptr jank_call9(jank_object_ptr f,
                             jank_object_ptr a1,
                             jank_object_ptr a2,
                             jank_object_ptr a3,
                             jank_object_ptr a4,
                             jank_object_ptr a5,
                             jank_object_ptr a6,
                             jank_object_ptr a7,
                             jank_object_ptr a8,
                             jank_object_ptr a9);
  jank_object_ptr jank_call10(jank_object_ptr f,
                              jank_object_ptr a1,
                              jank_object_ptr a2,
                              jank_object_ptr a3,
                              jank_object_ptr a4,
                              jank_object_ptr a5,
                              jank_object_ptr a6,
                              jank_object_ptr a7,
                              jank_object_ptr a8,
                              jank_object_ptr a9,
                              jank_object_ptr a10);
  jank_object_ptr jank_call11(jank_object_ptr f,
                              jank_object_ptr a1,
                              jank_object_ptr a2,
                              jank_object_ptr a3,
                              jank_object_ptr a4,
                              jank_object_ptr a5,
                              jank_object_ptr a6,
                              jank_object_ptr a7,
                              jank_object_ptr a8,
                              jank_object_ptr a9,
                              jank_object_ptr a10,
                              jank_object_ptr rest);

  jank_object_ptr jank_nil();
  jank_object_ptr jank_true();
  jank_object_ptr jank_false();
  jank_object_ptr jank_integer_create(jank_native_integer i);
  jank_object_ptr jank_real_create(jank_native_real r);
  jank_object_ptr jank_ratio_create(jank_native_integer numerator, jank_native_integer denominator);
  jank_object_ptr jank_string_create(char const *s);
  jank_object_ptr jank_symbol_create(jank_object_ptr ns, jank_object_ptr name);
  jank_object_ptr jank_character_create(char const *s);

  jank_object_ptr jank_list_create(uint64_t size, ...);
  jank_object_ptr jank_vector_create(uint64_t size, ...);
  jank_object_ptr jank_map_create(uint64_t pairs, ...);
  jank_object_ptr jank_set_create(uint64_t size, ...);

  jank_arity_flags jank_function_build_arity_flags(uint8_t highest_fixed_arity,
                                                   jank_native_bool is_variadic,
                                                   jank_native_bool is_variadic_ambiguous);
  jank_object_ptr jank_function_create(jank_arity_flags arity_flags);
  void jank_function_set_arity0(jank_object_ptr fn, jank_object_ptr (*f)());
  void jank_function_set_arity1(jank_object_ptr fn, jank_object_ptr (*f)(jank_object_ptr));
  void jank_function_set_arity2(jank_object_ptr fn,
                                jank_object_ptr (*f)(jank_object_ptr, jank_object_ptr));
  void
  jank_function_set_arity3(jank_object_ptr fn,
                           jank_object_ptr (*f)(jank_object_ptr, jank_object_ptr, jank_object_ptr));
  void jank_function_set_arity4(
    jank_object_ptr fn,
    jank_object_ptr (*f)(jank_object_ptr, jank_object_ptr, jank_object_ptr, jank_object_ptr));
  void jank_function_set_arity5(jank_object_ptr fn,
                                jank_object_ptr (*f)(jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr));
  void jank_function_set_arity6(jank_object_ptr fn,
                                jank_object_ptr (*f)(jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr));
  void jank_function_set_arity7(jank_object_ptr fn,
                                jank_object_ptr (*f)(jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr));
  void jank_function_set_arity8(jank_object_ptr fn,
                                jank_object_ptr (*f)(jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr));
  void jank_function_set_arity9(jank_object_ptr fn,
                                jank_object_ptr (*f)(jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr));
  void jank_function_set_arity10(jank_object_ptr fn,
                                 jank_object_ptr (*f)(jank_object_ptr,
                                                      jank_object_ptr,
                                                      jank_object_ptr,
                                                      jank_object_ptr,
                                                      jank_object_ptr,
                                                      jank_object_ptr,
                                                      jank_object_ptr,
                                                      jank_object_ptr,
                                                      jank_object_ptr,
                                                      jank_object_ptr));

  jank_object_ptr jank_closure_create(jank_arity_flags arity_flags, void *context);
  void jank_closure_set_arity0(jank_object_ptr fn, jank_object_ptr (*f)());
  void jank_closure_set_arity1(jank_object_ptr fn, jank_object_ptr (*f)(jank_object_ptr));
  void jank_closure_set_arity2(jank_object_ptr fn,
                               jank_object_ptr (*f)(jank_object_ptr, jank_object_ptr));
  void
  jank_closure_set_arity3(jank_object_ptr fn,
                          jank_object_ptr (*f)(jank_object_ptr, jank_object_ptr, jank_object_ptr));
  void jank_closure_set_arity4(
    jank_object_ptr fn,
    jank_object_ptr (*f)(jank_object_ptr, jank_object_ptr, jank_object_ptr, jank_object_ptr));
  void jank_closure_set_arity5(jank_object_ptr fn,
                               jank_object_ptr (*f)(jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr));
  void jank_closure_set_arity6(jank_object_ptr fn,
                               jank_object_ptr (*f)(jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr));
  void jank_closure_set_arity7(jank_object_ptr fn,
                               jank_object_ptr (*f)(jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr));
  void jank_closure_set_arity8(jank_object_ptr fn,
                               jank_object_ptr (*f)(jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr));
  void jank_closure_set_arity9(jank_object_ptr fn,
                               jank_object_ptr (*f)(jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr,
                                                    jank_object_ptr));
  void jank_closure_set_arity10(jank_object_ptr fn,
                                jank_object_ptr (*f)(jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr,
                                                     jank_object_ptr));

  jank_native_bool jank_truthy(jank_object_ptr o);
  jank_native_bool jank_equal(jank_object_ptr l, jank_object_ptr r);
  jank_native_hash jank_to_hash(jank_object_ptr o);

  void jank_set_meta(jank_object_ptr o, jank_object_ptr meta);

  void jank_throw(jank_object_ptr o);
  jank_object_ptr
  jank_try(jank_object_ptr try_fn, jank_object_ptr catch_fn, jank_object_ptr finally_fn);

  void jank_profile_enter(char const *label);
  void jank_profile_exit(char const *label);
  void jank_profile_report(char const *label);

#ifdef __cplusplus
}
#endif
