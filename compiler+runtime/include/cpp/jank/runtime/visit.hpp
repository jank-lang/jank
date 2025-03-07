#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/cons.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/obj/character.hpp>
#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/persistent_hash_set.hpp>
#include <jank/runtime/obj/persistent_sorted_set.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/persistent_array_map_sequence.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_hash_map_sequence.hpp>
#include <jank/runtime/obj/persistent_sorted_map.hpp>
#include <jank/runtime/obj/persistent_sorted_map_sequence.hpp>
#include <jank/runtime/obj/transient_hash_map.hpp>
#include <jank/runtime/obj/transient_sorted_map.hpp>
#include <jank/runtime/obj/transient_vector.hpp>
#include <jank/runtime/obj/transient_hash_set.hpp>
#include <jank/runtime/obj/transient_sorted_set.hpp>
#include <jank/runtime/obj/iterator.hpp>
#include <jank/runtime/obj/lazy_sequence.hpp>
#include <jank/runtime/obj/chunk_buffer.hpp>
#include <jank/runtime/obj/array_chunk.hpp>
#include <jank/runtime/obj/chunked_cons.hpp>
#include <jank/runtime/obj/range.hpp>
#include <jank/runtime/obj/integer_range.hpp>
#include <jank/runtime/obj/repeat.hpp>
#include <jank/runtime/obj/ratio.hpp>
#include <jank/runtime/obj/jit_function.hpp>
#include <jank/runtime/obj/jit_closure.hpp>
#include <jank/runtime/obj/multi_function.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/native_pointer_wrapper.hpp>
#include <jank/runtime/obj/persistent_vector_sequence.hpp>
#include <jank/runtime/obj/persistent_string_sequence.hpp>
#include <jank/runtime/obj/persistent_list_sequence.hpp>
#include <jank/runtime/obj/persistent_hash_set_sequence.hpp>
#include <jank/runtime/obj/persistent_sorted_set_sequence.hpp>
#include <jank/runtime/obj/native_array_sequence.hpp>
#include <jank/runtime/obj/native_vector_sequence.hpp>
#include <jank/runtime/obj/atom.hpp>
#include <jank/runtime/obj/volatile.hpp>
#include <jank/runtime/obj/delay.hpp>
#include <jank/runtime/obj/reduced.hpp>
#include <jank/runtime/obj/tagged_literal.hpp>
#include <jank/runtime/ns.hpp>
#include <jank/runtime/var.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core/to_string.hpp>

namespace jank::runtime
{
  template <typename T, typename F, typename... Args>
  requires behavior::object_like<T>
  [[gnu::hot]]
  constexpr auto visit_object(F const &fn, native_box<T const> const not_erased, Args &&...args)
  {
    return fn(const_cast<T *>(&not_erased->base), std::forward<Args>(args)...);
  }

  template <typename F, typename... Args>
  concept visitable = requires(F f) { f(obj::nil_ptr{}, std::declval<Args>()...); };

  template <typename F, typename... Args>
  requires visitable<F, Args...>
  [[gnu::hot]]
  auto visit_object(F const &fn, object const * const const_erased, Args &&...args)
  {
    assert(const_erased);
    auto * const erased(const_cast<object *>(const_erased));

    switch(erased->type)
    {
      case object_type::nil:
        {
          return fn(expect_object<obj::nil>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::boolean:
        {
          return fn(expect_object<obj::boolean>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::integer:
        {
          return fn(expect_object<obj::integer>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::real:
        {
          return fn(expect_object<obj::real>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_string:
        {
          return fn(expect_object<obj::persistent_string>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::keyword:
        {
          return fn(expect_object<obj::keyword>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::symbol:
        {
          return fn(expect_object<obj::symbol>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::character:
        {
          return fn(expect_object<obj::character>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_vector:
        {
          return fn(expect_object<obj::persistent_vector>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_list:
        {
          return fn(expect_object<obj::persistent_list>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_array_map:
        {
          return fn(expect_object<obj::persistent_array_map>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_array_map_sequence:
        {
          return fn(expect_object<obj::persistent_array_map_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_hash_map:
        {
          return fn(expect_object<obj::persistent_hash_map>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_hash_map_sequence:
        {
          return fn(expect_object<obj::persistent_hash_map_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_sorted_map:
        {
          return fn(expect_object<obj::persistent_sorted_map>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_sorted_map_sequence:
        {
          return fn(expect_object<obj::persistent_sorted_map_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::transient_hash_map:
        {
          return fn(expect_object<obj::transient_hash_map>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::transient_sorted_map:
        {
          return fn(expect_object<obj::transient_sorted_map>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::transient_vector:
        {
          return fn(expect_object<obj::transient_vector>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_hash_set:
        {
          return fn(expect_object<obj::persistent_hash_set>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_sorted_set:
        {
          return fn(expect_object<obj::persistent_sorted_set>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::transient_hash_set:
        {
          return fn(expect_object<obj::transient_hash_set>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::transient_sorted_set:
        {
          return fn(expect_object<obj::transient_sorted_set>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::cons:
        {
          return fn(expect_object<obj::cons>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::range:
        {
          return fn(expect_object<obj::range>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::integer_range:
        {
          return fn(expect_object<obj::integer_range>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::repeat:
        {
          return fn(expect_object<obj::repeat>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::ratio:
        {
          return fn(expect_object<obj::ratio>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::native_array_sequence:
        {
          return fn(expect_object<obj::native_array_sequence>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::native_vector_sequence:
        {
          return fn(expect_object<obj::native_vector_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_string_sequence:
        {
          return fn(expect_object<obj::persistent_string_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_vector_sequence:
        {
          return fn(expect_object<obj::persistent_vector_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_list_sequence:
        {
          return fn(expect_object<obj::persistent_list_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_hash_set_sequence:
        {
          return fn(expect_object<obj::persistent_hash_set_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_sorted_set_sequence:
        {
          return fn(expect_object<obj::persistent_sorted_set_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::iterator:
        {
          return fn(expect_object<obj::iterator>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::lazy_sequence:
        {
          return fn(expect_object<obj::lazy_sequence>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::chunk_buffer:
        {
          return fn(expect_object<obj::chunk_buffer>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::array_chunk:
        {
          return fn(expect_object<obj::array_chunk>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::chunked_cons:
        {
          return fn(expect_object<obj::chunked_cons>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::native_function_wrapper:
        {
          return fn(expect_object<obj::native_function_wrapper>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::native_pointer_wrapper:
        {
          return fn(expect_object<obj::native_pointer_wrapper>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::jit_function:
        {
          return fn(expect_object<obj::jit_function>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::jit_closure:
        {
          return fn(expect_object<obj::jit_closure>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::multi_function:
        {
          return fn(expect_object<obj::multi_function>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::atom:
        {
          return fn(expect_object<obj::atom>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::volatile_:
        {
          return fn(expect_object<obj::volatile_>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::reduced:
        {
          return fn(expect_object<obj::reduced>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::delay:
        {
          return fn(expect_object<obj::delay>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::ns:
        {
          return fn(expect_object<ns>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::var:
        {
          return fn(expect_object<var>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::var_thread_binding:
        {
          return fn(expect_object<var_thread_binding>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::var_unbound_root:
        {
          return fn(expect_object<var_unbound_root>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::tagged_literal:
        {
          return fn(expect_object<obj::tagged_literal>(erased), std::forward<Args>(args)...);
        }
        break;
      default:
        {
          util::string_builder sb;
          sb("invalid object type: ");
          sb(object_type_str(erased->type));
          sb(" raw value ");
          sb(static_cast<int>(erased->type));
          throw std::runtime_error{ sb.str() };
        }
        break;
    }
  }

  /* Allows the visiting of a single type. */
  template <typename T, typename F, typename... Args>
  [[gnu::hot]]
  constexpr auto visit_type(F const &fn, object const * const const_erased, Args &&...args)
  {
    if(const_erased->type == T::obj_type)
    {
      return fn(expect_object<T>(const_erased), std::forward<Args>(args)...);
    }
    else
    {
      throw std::runtime_error{ "invalid object type: "
                                + std::to_string(static_cast<int>(const_erased->type)) };
    }
  }

  template <typename F1, typename F2, typename... Args>
  requires(visitable<F1, Args...> && !std::convertible_to<F2, object const *>)
  [[gnu::hot]]
  constexpr auto
  visit_seqable(F1 const &fn, F2 const &else_fn, object const * const const_erased, Args &&...args)
  {
    assert(const_erased);
    auto * const erased(const_cast<object *>(const_erased));

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
    switch(erased->type)
    {
      case object_type::nil:
        {
          return fn(expect_object<obj::nil>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_vector:
        {
          return fn(expect_object<obj::persistent_vector>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_list:
        {
          return fn(expect_object<obj::persistent_list>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_array_map:
        {
          return fn(expect_object<obj::persistent_array_map>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_array_map_sequence:
        {
          return fn(expect_object<obj::persistent_array_map_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_hash_map:
        {
          return fn(expect_object<obj::persistent_hash_map>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_hash_map_sequence:
        {
          return fn(expect_object<obj::persistent_hash_map_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_sorted_map:
        {
          return fn(expect_object<obj::persistent_sorted_map>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_sorted_map_sequence:
        {
          return fn(expect_object<obj::persistent_sorted_map_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_hash_set:
        {
          return fn(expect_object<obj::persistent_hash_set>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_sorted_set:
        {
          return fn(expect_object<obj::persistent_sorted_set>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::cons:
        {
          return fn(expect_object<obj::cons>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::range:
        {
          return fn(expect_object<obj::range>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::integer_range:
        {
          return fn(expect_object<obj::integer_range>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::repeat:
        {
          return fn(expect_object<obj::repeat>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::native_array_sequence:
        {
          return fn(expect_object<obj::native_array_sequence>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::native_vector_sequence:
        {
          return fn(expect_object<obj::native_vector_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_string_sequence:
        {
          return fn(expect_object<obj::persistent_string_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_vector_sequence:
        {
          return fn(expect_object<obj::persistent_vector_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_list_sequence:
        {
          return fn(expect_object<obj::persistent_list_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_hash_set_sequence:
        {
          return fn(expect_object<obj::persistent_hash_set_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_sorted_set_sequence:
        {
          return fn(expect_object<obj::persistent_sorted_set_sequence>(erased),
                    std::forward<Args>(args)...);
        }
        break;
      case object_type::iterator:
        {
          return fn(expect_object<obj::iterator>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::lazy_sequence:
        {
          return fn(expect_object<obj::lazy_sequence>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::chunked_cons:
        {
          return fn(expect_object<obj::chunked_cons>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_string:
        {
          return fn(expect_object<obj::persistent_string>(erased), std::forward<Args>(args)...);
        }
        break;

      /* Not seqable. */
      default:
        return else_fn();
    }
#pragma clang diagnostic pop
  }

  /* Throws if the object isn't seqable. */
  template <typename F1, typename... Args>
  [[gnu::hot]]
  constexpr auto visit_seqable(F1 const &fn, object const * const const_erased, Args &&...args)
  {
    return visit_seqable(
      fn,
      [=]() -> decltype(fn(obj::cons_ptr{}, std::forward<Args>(args)...)) {
        throw std::runtime_error{ "not seqable: " + to_code_string(const_erased) };
      },
      const_erased,
      std::forward<Args>(args)...);
  }

  template <typename F, typename... Args>
  concept map_visitable
    = requires(F f) { f(obj::persistent_hash_map_ptr{}, std::declval<Args>()...); };

  template <typename F1, typename F2, typename... Args>
  requires(map_visitable<F1, Args...> && !std::convertible_to<F2, object const *>)
  [[gnu::hot]]
  constexpr auto
  visit_map_like(F1 const &fn, F2 const &else_fn, object const * const const_erased, Args &&...args)
  {
    assert(const_erased);
    auto * const erased(const_cast<object *>(const_erased));

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
    switch(erased->type)
    {
      case object_type::persistent_array_map:
        {
          return fn(expect_object<obj::persistent_array_map>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_hash_map:
        {
          return fn(expect_object<obj::persistent_hash_map>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_sorted_map:
        {
          return fn(expect_object<obj::persistent_sorted_map>(erased), std::forward<Args>(args)...);
        }
        break;
      /* Not map-like. */
      default:
        return else_fn();
    }
#pragma clang diagnostic pop
  }

  /* Throws if the object isn't map-like. */
  template <typename F1, typename... Args>
  [[gnu::hot]]
  constexpr auto visit_map_like(F1 const &fn, object const * const const_erased, Args &&...args)
  {
    return visit_map_like(
      fn,
      [=]() -> decltype(fn(obj::persistent_hash_map_ptr{}, std::forward<Args>(args)...)) {
        throw std::runtime_error{ "not map-like: " + to_code_string(const_erased) };
      },
      const_erased,
      std::forward<Args>(args)...);
  }

  template <typename F1, typename F2, typename... Args>
  requires(visitable<F1, Args...> && !std::convertible_to<F2, object const *>)
  [[gnu::hot]]
  constexpr auto
  visit_set_like(F1 const &fn, F2 const &else_fn, object const * const const_erased, Args &&...args)
  {
    assert(const_erased);
    auto * const erased(const_cast<object *>(const_erased));

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
    switch(erased->type)
    {
      case object_type::persistent_hash_set:
        {
          return fn(expect_object<obj::persistent_hash_set>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::persistent_sorted_set:
        {
          return fn(expect_object<obj::persistent_sorted_set>(erased), std::forward<Args>(args)...);
        }
        break;
      /* Not set-like. */
      default:
        return else_fn();
    }
#pragma clang diagnostic pop
  }

  /* Throws if the object isn't set-like. */
  template <typename F1, typename... Args>
  [[gnu::hot]]
  constexpr auto visit_set_like(F1 const &fn, object const * const const_erased, Args &&...args)
  {
    return visit_set_like(
      fn,
      [=]() -> decltype(fn(obj::persistent_hash_set_ptr{}, std::forward<Args>(args)...)) {
        throw std::runtime_error{ "not set-like: " + to_code_string(const_erased) };
      },
      const_erased,
      std::forward<Args>(args)...);
  }

  template <typename F1, typename F2, typename... Args>
  requires(!std::convertible_to<F2, object const *>)
  [[gnu::hot]]
  constexpr auto visit_number_like(F1 const &fn,
                                   F2 const &else_fn,
                                   object const * const const_erased,
                                   Args &&...args)
  {
    assert(const_erased);
    auto * const erased(const_cast<object *>(const_erased));

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
    switch(erased->type)
    {
      case object_type::integer:
        {
          return fn(expect_object<obj::integer>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::real:
        {
          return fn(expect_object<obj::real>(erased), std::forward<Args>(args)...);
        }
        break;
      case object_type::ratio:
        {
          return fn(expect_object<obj::ratio>(erased), std::forward<Args>(args)...);
        }
        break;
      default:
        return else_fn();
    }
#pragma clang diagnostic pop
  }

  /* Throws if the object isn't number-like. */
  template <typename F1, typename... Args>
  [[gnu::hot]]
  constexpr auto visit_number_like(F1 const &fn, object const * const const_erased, Args &&...args)
  {
    return visit_number_like(
      fn,
      [=]() -> decltype(fn(obj::integer_ptr{}, std::forward<Args>(args)...)) {
        throw std::runtime_error{ "not a number: " + to_code_string(const_erased) };
      },
      const_erased,
      std::forward<Args>(args)...);
  }
}
