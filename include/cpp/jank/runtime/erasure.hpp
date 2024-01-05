#pragma once

#include <magic_enum.hpp>

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/cons.hpp>
#include <jank/runtime/obj/string.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/vector.hpp>
#include <jank/runtime/obj/list.hpp>
#include <jank/runtime/obj/set.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/persistent_array_map_sequence.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_hash_map_sequence.hpp>
#include <jank/runtime/obj/iterator.hpp>
#include <jank/runtime/obj/range.hpp>
#include <jank/runtime/obj/jit_function.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/persistent_vector_sequence.hpp>
#include <jank/runtime/obj/persistent_list_sequence.hpp>
#include <jank/runtime/obj/persistent_set_sequence.hpp>
#include <jank/runtime/obj/native_array_sequence.hpp>
#include <jank/runtime/obj/native_vector_sequence.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/var.hpp>

namespace jank::runtime
{
  /* Most of the system is polymorphic using type-erased objects. Given any object, an erase call
   * will get you what you need. If you don't need to type-erase, though, don't! */
  template <typename T>
  requires behavior::objectable<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr object_ptr erase(native_box<T> const o)
  { return &o->base; }
  template <typename T>
  requires behavior::objectable<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr object_ptr erase(native_box<T const> const o)
  { return const_cast<object_ptr>(&o->base); }
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr object_ptr erase(object_ptr const o)
  { return o; }
  template <typename T>
  requires behavior::objectable<std::decay_t<std::remove_pointer_t<T>>>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr object_ptr erase(T const o)
  { return const_cast<object*>(&o->base); }

  /* This is dangerous. You probably don't want it. Just use `visit_object`. However, if you're
   * absolutely certain you know the type of an erased object, I guess you can use this. */
  template <typename T>
  requires behavior::objectable<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr native_box<T> expect_object(object_ptr const o)
  {
    assert(o);
    assert(o->type == detail::object_type_to_enum<T>::value);
    return reinterpret_cast<T*>(reinterpret_cast<char*>(o.data) - offsetof(T, base));
  }
  template <typename T>
  requires behavior::objectable<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr native_box<T> expect_object(object const * const o)
  {
    assert(o);
    assert(o->type == detail::object_type_to_enum<T>::value);
    return reinterpret_cast<T*>(reinterpret_cast<char*>(const_cast<object*>(o)) - offsetof(T, base));
  }

  template <typename T, typename F, typename ...Args>
  requires behavior::objectable<T>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr auto visit_object(F &&fn, native_box<T const> const not_erased, Args && ...args)
  { return fn(const_cast<T*>(&not_erased->base), std::forward<Args>(args)...); }

  template <typename F, typename ...Args>
  concept visitable = requires(F f)
  {
    f(obj::nil_ptr{}, std::declval<Args>()...);
  };

  template <typename F, typename ...Args>
  requires visitable<F, Args...>
  [[gnu::always_inline, gnu::flatten, gnu::hot]]
  constexpr auto visit_object(F &&fn, object const * const const_erased, Args && ...args)
  {
    assert(const_erased);
    auto * const erased(const_cast<object*>(const_erased));

    switch(erased->type)
    {
      case object_type::nil:
      {
        return fn(expect_object<obj::nil>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::boolean:
      {
        return fn(expect_object<obj::boolean>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::integer:
      {
        return fn(expect_object<obj::integer>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::real:
      {
        return fn(expect_object<obj::real>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::string:
      {
        return fn(expect_object<obj::string>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::keyword:
      {
        return fn(expect_object<obj::keyword>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::symbol:
      {
        return fn(expect_object<obj::symbol>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::vector:
      {
        return fn(expect_object<obj::vector>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::list:
      {
        return fn(expect_object<obj::list>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::persistent_array_map:
      {
        return fn(expect_object<obj::persistent_array_map>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::persistent_array_map_sequence:
      {
        return fn(expect_object<obj::persistent_array_map_sequence>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::persistent_hash_map:
      {
        return fn(expect_object<obj::persistent_hash_map>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::persistent_hash_map_sequence:
      {
        return fn(expect_object<obj::persistent_hash_map_sequence>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::set:
      {
        return fn(expect_object<obj::set>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::cons:
      {
        return fn(expect_object<obj::cons>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::range:
      {
        return fn(expect_object<obj::range>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::native_array_sequence:
      {
        return fn(expect_object<obj::native_array_sequence>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::native_vector_sequence:
      {
        return fn(expect_object<obj::native_vector_sequence>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::persistent_vector_sequence:
      {
        return fn(expect_object<obj::persistent_vector_sequence>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::persistent_list_sequence:
      {
        return fn(expect_object<obj::persistent_list_sequence>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::persistent_set_sequence:
      {
        return fn(expect_object<obj::persistent_set_sequence>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::iterator:
      {
        return fn(expect_object<obj::iterator>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::native_function_wrapper:
      {
        return fn(expect_object<obj::native_function_wrapper>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::jit_function:
      {
        return fn(expect_object<obj::jit_function>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::ns:
      {
        return fn(expect_object<ns>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::var:
      {
        return fn(expect_object<var>(erased), std::forward<Args>(args)...);
      } break;
      case object_type::var_thread_binding:
      {
        return fn(expect_object<var_thread_binding>(erased), std::forward<Args>(args)...);
      } break;
      default:
      {
        //throw std::runtime_error
        //{
        //  fmt::format
        //  (
        //    "invalid object type: {} raw value {}",
        //    magic_enum::enum_name(erased->type),
        //    static_cast<int>(erased->type)
        //  )
        //};
      } break;
    }
  }
}
