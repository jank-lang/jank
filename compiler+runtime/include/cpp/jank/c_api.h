#pragma once

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

  jank_object_ptr jank_var_intern(jank_object_ptr ns, jank_object_ptr name);
  jank_object_ptr jank_var_bind_root(jank_object_ptr var, jank_object_ptr val);

  jank_object_ptr jank_keyword_intern(jank_object_ptr ns, jank_object_ptr name);

  jank_object_ptr jank_deref(jank_object_ptr o);

  jank_object_ptr jank_call0(jank_object_ptr f);
  jank_object_ptr jank_call1(jank_object_ptr f, jank_object_ptr a1);
  jank_object_ptr jank_call2(jank_object_ptr f, jank_object_ptr a1, jank_object_ptr a2);
  jank_object_ptr
  jank_call3(jank_object_ptr f, jank_object_ptr a1, jank_object_ptr a2, jank_object_ptr a3);

  jank_object_ptr jank_nil();
  jank_object_ptr jank_true();
  jank_object_ptr jank_false();
  jank_object_ptr jank_integer_create(jank_native_integer i);
  jank_object_ptr jank_real_create(jank_native_real r);
  jank_object_ptr jank_string_create(char const *s);
  jank_object_ptr jank_symbol_create(jank_object_ptr ns, jank_object_ptr name);
  jank_object_ptr jank_character_create(char const *s);

  jank_arity_flags jank_function_build_arity_flags(uint8_t highest_fixed_arity,
                                                   jank_native_bool is_variadic,
                                                   jank_native_bool is_variadic_ambiguous);
  jank_object_ptr jank_function_create(jank_arity_flags arity_flags);
  jank_object_ptr jank_function_set_arity0(jank_object_ptr fn, jank_object_ptr (*f)());
  jank_object_ptr
  jank_function_set_arity1(jank_object_ptr fn, jank_object_ptr (*f)(jank_object_ptr));
  jank_object_ptr jank_function_set_arity2(jank_object_ptr fn,
                                           jank_object_ptr (*f)(jank_object_ptr, jank_object_ptr));
  jank_object_ptr
  jank_function_set_arity3(jank_object_ptr fn,
                           jank_object_ptr (*f)(jank_object_ptr, jank_object_ptr, jank_object_ptr));

  jank_native_bool jank_truthy(jank_object_ptr o);
  jank_native_bool jank_equal(jank_object_ptr l, jank_object_ptr r);
  jank_native_hash jank_to_hash(jank_object_ptr o);

  void jank_throw(jank_object_ptr o);

#ifdef __cplusplus
}
#endif
