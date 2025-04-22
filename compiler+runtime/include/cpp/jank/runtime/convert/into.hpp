#pragma once

#include <jank/runtime/convert.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/core/make_box.hpp>

namespace jank::runtime
{
  template <>
  struct convert<void, object_ref>
  {
    static constexpr object_ref call()
    {
      return jank_nil;
    }
  };

  template <typename T>
  requires behavior::object_like<T>
  struct convert<oref<T>, object_ref>
  {
    static constexpr object_ref call(oref<T> const o)
    {
      return o;
    }
  };

  template <typename T>
  requires behavior::object_like<T>
  struct convert<T *, object_ref>
  {
    static object_ref call(T * const o)
    {
      return o;
    }
  };

  template <>
  struct convert<object *, object_ref>
  {
    static constexpr object_ref call(object * const o)
    {
      return o;
    }
  };

  template <>
  struct convert<object const *, object_ref>
  {
    static object_ref call(object const * const o)
    {
      return const_cast<object *>(o);
    }
  };

  template <>
  struct convert<bool, object_ref>
  {
    static constexpr object_ref call(bool const o)
    {
      return make_box(o);
    }
  };

  /* Native integer primitives. */
  template <typename Input>
  requires(std::is_integral_v<Input>
           && !jtl::is_any_same<bool, char, char8_t, char16_t, char32_t, wchar_t>)
  struct convert<Input, object_ref>
  {
    static constexpr object_ref call(Input const o)
    {
      return make_box(o);
    }
  };

  /* Native floating point primitives. */
  template <typename Input>
  requires(std::is_floating_point_v<Input>)
  struct convert<Input, object_ref>
  {
    static constexpr object_ref call(Input const o)
    {
      return make_box(o);
    }
  };

  /* Native strings. */
  template <typename Input>
  requires(
    jtl::is_any_same<Input, jtl::immutable_string, native_persistent_string_view, std::string>)
  struct convert<Input, object_ref>
  {
    static constexpr object_ref call(Input const &o)
    {
      return make_box(o);
    }

    static constexpr object_ref call(Input &&o)
    {
      return make_box(std::move(o));
    }
  };

  template <template <typename> typename V, typename Input>
  requires(
    convertible<Input, object_ref>
    && (jtl::is_same<V<Input>, native_vector<Input>> || jtl::is_same<V<Input>, std::vector<Input>>))
  struct convert<V<Input>, object_ref>
  {
    static object_ref call(V<Input> const &o)
    {
      runtime::detail::native_transient_vector trans;
      for(auto const &e : o)
      {
        trans.push_back(convert<Input, object_ref>::call(e));
      }
      return make_box<obj::persistent_vector>(trans);
    }
  };
}
