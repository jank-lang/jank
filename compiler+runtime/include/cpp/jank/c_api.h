#pragma once

/* NOLINTNEXTLINE(modernize-deprecated-headers) */
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /* NOLINTNEXTLINE(modernize-use-using) */
  typedef void *jank_object_ref;
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

  jank_object_ref jank_eval(jank_object_ref s);
  /* TODO: Add jank_read_c_string to avoid boxing so much. */
  jank_object_ref jank_read_string(jank_object_ref s);

  void jank_ns_set_symbol_counter(char const * const ns, uint64_t const count);

  jank_object_ref jank_var_intern(jank_object_ref ns, jank_object_ref name);
  jank_object_ref jank_var_bind_root(jank_object_ref var, jank_object_ref val);
  jank_object_ref jank_var_set_dynamic(jank_object_ref var, jank_object_ref dynamic);

  jank_object_ref jank_keyword_intern(jank_object_ref ns, jank_object_ref name);

  jank_object_ref jank_deref(jank_object_ref o);

  jank_object_ref jank_call0(jank_object_ref f);
  jank_object_ref jank_call1(jank_object_ref f, jank_object_ref a1);
  jank_object_ref jank_call2(jank_object_ref f, jank_object_ref a1, jank_object_ref a2);
  jank_object_ref
  jank_call3(jank_object_ref f, jank_object_ref a1, jank_object_ref a2, jank_object_ref a3);
  jank_object_ref jank_call4(jank_object_ref f,
                             jank_object_ref a1,
                             jank_object_ref a2,
                             jank_object_ref a3,
                             jank_object_ref a4);
  jank_object_ref jank_call5(jank_object_ref f,
                             jank_object_ref a1,
                             jank_object_ref a2,
                             jank_object_ref a3,
                             jank_object_ref a4,
                             jank_object_ref a5);
  jank_object_ref jank_call6(jank_object_ref f,
                             jank_object_ref a1,
                             jank_object_ref a2,
                             jank_object_ref a3,
                             jank_object_ref a4,
                             jank_object_ref a5,
                             jank_object_ref a6);
  jank_object_ref jank_call7(jank_object_ref f,
                             jank_object_ref a1,
                             jank_object_ref a2,
                             jank_object_ref a3,
                             jank_object_ref a4,
                             jank_object_ref a5,
                             jank_object_ref a6,
                             jank_object_ref a7);
  jank_object_ref jank_call8(jank_object_ref f,
                             jank_object_ref a1,
                             jank_object_ref a2,
                             jank_object_ref a3,
                             jank_object_ref a4,
                             jank_object_ref a5,
                             jank_object_ref a6,
                             jank_object_ref a7,
                             jank_object_ref a8);
  jank_object_ref jank_call9(jank_object_ref f,
                             jank_object_ref a1,
                             jank_object_ref a2,
                             jank_object_ref a3,
                             jank_object_ref a4,
                             jank_object_ref a5,
                             jank_object_ref a6,
                             jank_object_ref a7,
                             jank_object_ref a8,
                             jank_object_ref a9);
  jank_object_ref jank_call10(jank_object_ref f,
                              jank_object_ref a1,
                              jank_object_ref a2,
                              jank_object_ref a3,
                              jank_object_ref a4,
                              jank_object_ref a5,
                              jank_object_ref a6,
                              jank_object_ref a7,
                              jank_object_ref a8,
                              jank_object_ref a9,
                              jank_object_ref a10);
  jank_object_ref jank_call11(jank_object_ref f,
                              jank_object_ref a1,
                              jank_object_ref a2,
                              jank_object_ref a3,
                              jank_object_ref a4,
                              jank_object_ref a5,
                              jank_object_ref a6,
                              jank_object_ref a7,
                              jank_object_ref a8,
                              jank_object_ref a9,
                              jank_object_ref a10,
                              jank_object_ref rest);

  jank_object_ref jank_nil();
  jank_object_ref jank_true();
  jank_object_ref jank_false();
  jank_object_ref jank_integer_create(jank_native_integer i);
  jank_object_ref jank_real_create(jank_native_real r);
  jank_object_ref jank_ratio_create(jank_native_integer numerator, jank_native_integer denominator);
  jank_object_ref jank_string_create(char const *s);
  jank_object_ref jank_symbol_create(jank_object_ref ns, jank_object_ref name);
  jank_object_ref jank_character_create(char const *s);

  jank_object_ref jank_list_create(uint64_t size, ...);
  jank_object_ref jank_vector_create(uint64_t size, ...);
  jank_object_ref jank_map_create(uint64_t pairs, ...);
  jank_object_ref jank_set_create(uint64_t size, ...);

  jank_arity_flags jank_function_build_arity_flags(uint8_t highest_fixed_arity,
                                                   jank_native_bool is_variadic,
                                                   jank_native_bool is_variadic_ambiguous);
  jank_object_ref jank_function_create(jank_arity_flags arity_flags);
  void jank_function_set_arity0(jank_object_ref fn, jank_object_ref (*f)());
  void jank_function_set_arity1(jank_object_ref fn, jank_object_ref (*f)(jank_object_ref));
  void jank_function_set_arity2(jank_object_ref fn,
                                jank_object_ref (*f)(jank_object_ref, jank_object_ref));
  void
  jank_function_set_arity3(jank_object_ref fn,
                           jank_object_ref (*f)(jank_object_ref, jank_object_ref, jank_object_ref));
  void jank_function_set_arity4(
    jank_object_ref fn,
    jank_object_ref (*f)(jank_object_ref, jank_object_ref, jank_object_ref, jank_object_ref));
  void jank_function_set_arity5(jank_object_ref fn,
                                jank_object_ref (*f)(jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref));
  void jank_function_set_arity6(jank_object_ref fn,
                                jank_object_ref (*f)(jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref));
  void jank_function_set_arity7(jank_object_ref fn,
                                jank_object_ref (*f)(jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref));
  void jank_function_set_arity8(jank_object_ref fn,
                                jank_object_ref (*f)(jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref));
  void jank_function_set_arity9(jank_object_ref fn,
                                jank_object_ref (*f)(jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref));
  void jank_function_set_arity10(jank_object_ref fn,
                                 jank_object_ref (*f)(jank_object_ref,
                                                      jank_object_ref,
                                                      jank_object_ref,
                                                      jank_object_ref,
                                                      jank_object_ref,
                                                      jank_object_ref,
                                                      jank_object_ref,
                                                      jank_object_ref,
                                                      jank_object_ref,
                                                      jank_object_ref));

  jank_object_ref jank_closure_create(jank_arity_flags arity_flags, void *context);
  void jank_closure_set_arity0(jank_object_ref fn, jank_object_ref (*f)());
  void jank_closure_set_arity1(jank_object_ref fn, jank_object_ref (*f)(jank_object_ref));
  void jank_closure_set_arity2(jank_object_ref fn,
                               jank_object_ref (*f)(jank_object_ref, jank_object_ref));
  void
  jank_closure_set_arity3(jank_object_ref fn,
                          jank_object_ref (*f)(jank_object_ref, jank_object_ref, jank_object_ref));
  void jank_closure_set_arity4(
    jank_object_ref fn,
    jank_object_ref (*f)(jank_object_ref, jank_object_ref, jank_object_ref, jank_object_ref));
  void jank_closure_set_arity5(jank_object_ref fn,
                               jank_object_ref (*f)(jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref));
  void jank_closure_set_arity6(jank_object_ref fn,
                               jank_object_ref (*f)(jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref));
  void jank_closure_set_arity7(jank_object_ref fn,
                               jank_object_ref (*f)(jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref));
  void jank_closure_set_arity8(jank_object_ref fn,
                               jank_object_ref (*f)(jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref));
  void jank_closure_set_arity9(jank_object_ref fn,
                               jank_object_ref (*f)(jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref,
                                                    jank_object_ref));
  void jank_closure_set_arity10(jank_object_ref fn,
                                jank_object_ref (*f)(jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref,
                                                     jank_object_ref));

  jank_native_bool jank_truthy(jank_object_ref o);
  jank_native_bool jank_equal(jank_object_ref l, jank_object_ref r);
  jank_native_hash jank_to_hash(jank_object_ref o);
  jank_native_integer jank_to_integer(jank_object_ref o);
  jank_native_integer jank_shift_mask_case_integer(jank_object_ref o,
                                                   jank_native_integer shift,
                                                   jank_native_integer mask);

  void jank_set_meta(jank_object_ref o, jank_object_ref meta);

  void jank_throw(jank_object_ref o);
  jank_object_ref
  jank_try(jank_object_ref try_fn, jank_object_ref catch_fn, jank_object_ref finally_fn);

  void jank_profile_enter(char const *label);
  void jank_profile_exit(char const *label);
  void jank_profile_report(char const *label);

#ifdef __cplusplus
}
#endif
