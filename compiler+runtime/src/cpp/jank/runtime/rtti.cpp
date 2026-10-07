#include <jank/runtime/rtti.hpp>

namespace jank::runtime
{
  /* These RTTI specializations for small integers and small reals are specifically for
   * actual small_integer_ref and small_real_ref objects. So we end up grabbing the
   * pointer out of these and casting, rather than pulling out tagged numbers. */
  template <>
  obj::small_integer_ref expect_object<obj::small_integer>(object_ref const o)
  {
    jank_debug_assert(o.is_some());
    jank_debug_assert(o.get_type() == object_type::small_integer);

    return static_cast<obj::small_integer *>(o.ptr())->data;
  }

  template <>
  obj::small_real_ref expect_object<obj::small_real>(object_ref const o)
  {
    jank_debug_assert(o.is_some());
    jank_debug_assert(o.get_type() == object_type::small_real);

    return static_cast<obj::small_real *>(o.ptr())->data;
  }

  template <>
  obj::small_integer_ref try_object<obj::small_integer>(object_ref const o)
  {
    if(o.get_type() != object_type::small_integer)
    {
      jtl::string_builder sb;
      sb("An object of type `");
      sb(object_type_str(object_type::small_integer));
      sb("` was expected here, but a `");
      sb(object_type_str(o.get_type()));
      sb("` was provided instead.");
      throw std::runtime_error{ sb.str() };
    }
    return static_cast<obj::small_integer *>(o.ptr())->data;
  }

  template <>
  obj::small_real_ref try_object<obj::small_real>(object_ref const o)
  {
    if(o.get_type() != object_type::small_real)
    {
      jtl::string_builder sb;
      sb("An object of type `");
      sb(object_type_str(object_type::small_real));
      sb("` was expected here, but a `");
      sb(object_type_str(o.get_type()));
      sb("` was provided instead.");
      throw std::runtime_error{ sb.str() };
    }
    return static_cast<obj::small_real *>(o.ptr())->data;
  }

  template <>
  obj::small_integer_ref expect_object<obj::small_integer>(object const * const o)
  {
    jank_debug_assert(o);
    jank_debug_assert(o->type == object_type::small_integer);

    return static_cast<obj::small_integer *>(const_cast<object *>(o))->data;
  }

  template <>
  obj::small_real_ref expect_object<obj::small_real>(object const * const o)
  {
    jank_debug_assert(o);
    jank_debug_assert(o->type == object_type::small_real);

    return static_cast<obj::small_real *>(const_cast<object *>(o))->data;
  }
}
