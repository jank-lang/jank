#pragma once

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <jtl/immutable_string.hpp>
#include <jtl/trait/type_name.hpp>

#include <jank/runtime/convert/builtin.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/opaque_box.hpp>

namespace jank::runtime
{
  template <typename T>
  struct always_object_ref
  {
    using type = object_ref const;
  };

  template <typename R, typename... Args>
  auto convert_function(R (* const fn)(Args...))
  {
    if constexpr(std::conjunction_v<std::is_same<object_ref, R>,
                                    std::is_same<object_ref const, Args>...>)
    {
      return fn;
    }
    else
    {
      return std::function<object_ref(typename always_object_ref<Args>::type...)>{
        [fn](Args &&...args) -> object_ref {
          if constexpr(jtl::is_void<R>)
          {
            fn(convert<jtl::decay_t<Args>>::into_object(jtl::forward<Args>(args))...);
            return convert<R>::into_object();
          }
          else
          {
            return convert<R>::into_object(
              fn(convert<jtl::decay_t<Args>>::into_object(jtl::forward<Args>(args))...));
          }
        }
      };
    }
  }

  namespace detail
  {
    /* Build a type name string of the form "Foo*" for use in opaque boxes.
     * This matches the canonical type names the jank compiler uses when
     * boxing/unboxing pointers. Const qualifiers are intentionally stripped,
     * since the compiler's `get_qualified_type_name` also drops them when
     * naming pointer types. */
    template <typename T>
    jtl::immutable_string opaque_box_type_name()
    {
      using Bare = std::remove_cv_t<std::remove_reference_t<T>>;
      constexpr auto tn{ jtl::type_name<Bare>() };
      std::string name{ tn.data(), tn.size() };
      name += "*";
      return { name.c_str() };
    }

    /* Convert a native argument (which may be a reference) into an object_ref,
     * suitable for passing through `object::call`.
     *
     * If the type is already trait-convertible, use the normal conversion.
     * Otherwise, fall back to boxing a pointer to the value in an opaque_box,
     * which matches how `cpp/box` works on raw pointers. */
    template <typename T>
    object_ref wrap_arg(T &&arg)
    {
      using Bare = std::remove_cv_t<std::remove_reference_t<T>>;
      if constexpr(convertible_into_object<Bare>)
      {
        return convert<Bare>::into_object(std::forward<T>(arg));
      }
      else
      {
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) */
        return make_box<obj::opaque_box>(const_cast<Bare *>(std::addressof(arg)),
                                         opaque_box_type_name<T>());
      }
    }

    /* Convert an object_ref into a native return value.
     *
     * For void, we simply return. For types which are trait-convertible, we
     * use the normal from_object. For other reference/pointer types we
     * expect the jank fn to return an opaque_box wrapping a pointer and
     * we dereference it. */
    template <typename R>
    decltype(auto) unwrap_result(object_ref const o)
    {
      if constexpr(std::is_void_v<R>)
      {
        return;
      }
      else if constexpr(convertible_from_object<std::remove_cvref_t<R>>)
      {
        return convert<std::remove_cvref_t<R>>::from_object(o);
      }
      else if constexpr(std::is_pointer_v<std::remove_reference_t<R>>)
      {
        using Ptr = std::remove_reference_t<R>;
        auto const box{ try_object<obj::opaque_box>(o) };
        return static_cast<Ptr>(box->data);
      }
      else if constexpr(std::is_lvalue_reference_v<R>)
      {
        using Bare = std::remove_cv_t<std::remove_reference_t<R>>;
        auto const box{ try_object<obj::opaque_box>(o) };
        return static_cast<R>(*static_cast<Bare *>(box->data));
      }
      else
      {
        static_assert(
          convertible_from_object<std::remove_cvref_t<R>>,
          "jank is unable to convert an object_ref into the requested std::function return "
          "type. Either provide a convert<> specialization for it or return a trait-convertible "
          "type.");
      }
    }
  }

  /* Conversion from any callable jank object into a std::function.
   *
   * When a jank fn (or any other callable object) is passed to a C++ fn
   * expecting a std::function<R(Args...)>, the analyzer sees that this
   * specialization exists and marks the call as needing a `from_object`
   * trait conversion. The codegen then emits a call to
   * `jank::runtime::convert<std::function<R(Args...)>>>::from_object(fn)`.
   *
   * The returned std::function captures the object_ref and forwards any
   * native arguments by converting them to object_refs using `wrap_arg`.
   * Return values are converted back using `unwrap_result`. */
  template <typename R, typename... Args>
  struct convert<std::function<R(Args...)>>
  {
    static std::function<R(Args...)> from_object(object_ref const o)
    {
      if(!is_callable(o))
      {
        throw std::runtime_error{ "Cannot convert non-callable jank object to std::function." };
      }

      return [o](Args... args) -> R {
        /* We need to dispatch on the exact number of arguments, since
         * `object::call` has overloads for each arity. */
        if constexpr(std::is_void_v<R>)
        {
          o.call(detail::wrap_arg<Args>(std::forward<Args>(args))...);
          return;
        }
        else
        {
          return detail::unwrap_result<R>(
            o.call(detail::wrap_arg<Args>(std::forward<Args>(args))...));
        }
      };
    }

    static object_ref into_object(std::function<R(Args...)> const &f)
    {
      return make_box<obj::native_function_wrapper>(obj::detail::function_type{ f });
    }
  };
}
