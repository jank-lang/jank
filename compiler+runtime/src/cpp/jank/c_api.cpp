#include <jank/c_api.h>

using namespace jank;
using namespace jank::runtime;

extern "C"
{
  jank_object_ptr jank_eval(jank_object_ptr s)
  {
    auto const s_obj(try_object<obj::persistent_string>(reinterpret_cast<object *>(s)));
    return __rt_ctx->eval_string(s_obj->data);
  }

  jank_object_ptr jank_read_string(jank_object_ptr s)
  {
    auto const s_obj(try_object<obj::persistent_string>(reinterpret_cast<object *>(s)));
    return __rt_ctx->read_string(s_obj->data);
  }

  jank_object_ptr jank_var_intern(jank_object_ptr ns, jank_object_ptr name)
  {
    auto const ns_obj(try_object<obj::persistent_string>(reinterpret_cast<object *>(ns)));
    auto const name_obj(try_object<obj::persistent_string>(reinterpret_cast<object *>(name)));
    return erase(__rt_ctx->intern_var(ns_obj->data, name_obj->data).expect_ok());
  }

  jank_object_ptr jank_var_bind_root(jank_object_ptr var, jank_object_ptr val)
  {
    auto const var_obj(try_object<runtime::var>(reinterpret_cast<object *>(var)));
    auto const val_obj(reinterpret_cast<object *>(val));
    return erase(var_obj->bind_root(val_obj));
  }

  jank_object_ptr jank_deref(jank_object_ptr o)
  {
    auto const o_obj(reinterpret_cast<object *>(o));
    return behavior::deref(o_obj);
  }

  jank_object_ptr jank_call0(jank_object_ptr f)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    return dynamic_call(f_obj);
  }

  jank_object_ptr jank_call1(jank_object_ptr f, jank_object_ptr a1)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    auto const a1_obj(reinterpret_cast<object *>(a1));
    return dynamic_call(f_obj, a1_obj);
  }

  jank_object_ptr jank_call2(jank_object_ptr f, jank_object_ptr a1, jank_object_ptr a2)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    auto const a1_obj(reinterpret_cast<object *>(a1));
    auto const a2_obj(reinterpret_cast<object *>(a2));
    return dynamic_call(f_obj, a1_obj, a2_obj);
  }

  jank_object_ptr
  jank_call3(jank_object_ptr f, jank_object_ptr a1, jank_object_ptr a2, jank_object_ptr a3)
  {
    auto const f_obj(reinterpret_cast<object *>(f));
    auto const a1_obj(reinterpret_cast<object *>(a1));
    auto const a2_obj(reinterpret_cast<object *>(a2));
    auto const a3_obj(reinterpret_cast<object *>(a3));
    return dynamic_call(f_obj, a1_obj, a2_obj, a3_obj);
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

  jank_object_ptr jank_create_integer(jank_native_integer i)
  {
    return erase(make_box(i));
  }

  jank_object_ptr jank_create_real(jank_native_real r)
  {
    return erase(make_box(r));
  }

  jank_object_ptr jank_create_string(char const *s)
  {
    assert(s);
    return erase(make_box(s));
  }

  jank_object_ptr jank_create_function0(jank_object_ptr (*f)())
  {
    return erase(make_box<obj::native_function_wrapper>(f));
  }

  jank_object_ptr jank_create_function1(jank_object_ptr (*f)(jank_object_ptr))
  {
    return erase(make_box<obj::native_function_wrapper>(f));
  }

  jank_object_ptr jank_create_function2(jank_object_ptr (*f)(jank_object_ptr, jank_object_ptr))
  {
    return erase(make_box<obj::native_function_wrapper>(f));
  }

  jank_object_ptr
  jank_create_function3(jank_object_ptr (*f)(jank_object_ptr, jank_object_ptr, jank_object_ptr))
  {
    return erase(make_box<obj::native_function_wrapper>(f));
  }

  jank_native_bool jank_truthy(jank_object_ptr o)
  {
    auto const o_obj(reinterpret_cast<object *>(o));
    return static_cast<jank_native_bool>(truthy(o_obj));
  }

  jank_native_bool jank_equal(jank_object_ptr l, jank_object_ptr r)
  {
    auto const l_obj(reinterpret_cast<object *>(l));
    auto const r_obj(reinterpret_cast<object *>(r));
    return static_cast<jank_native_bool>(equal(l_obj, r_obj));
  }

  jank_native_hash jank_to_hash(jank_object_ptr o)
  {
    auto const o_obj(reinterpret_cast<object *>(o));
    return to_hash(o_obj);
  }
}
