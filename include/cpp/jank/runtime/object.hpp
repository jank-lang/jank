#pragma once

#include <concepts>

#include <fmt/format.h>

/* TODO: Move to obj namespace */
namespace jank::runtime
{
  enum class object_type
  {
    nil = 1,
    boolean,
    integer,
    real,
    string,
    keyword,
    symbol,
    list,
    vector,
    persistent_array_map,
    persistent_array_map_sequence,
    persistent_hash_map,
    persistent_hash_map_sequence,
    set,
    cons,
    range,
    iterator,
    native_function_wrapper,
    jit_function,
    native_array_sequence,
    native_vector_sequence,
    persistent_vector_sequence,
    persistent_list_sequence,
    persistent_set_sequence,
    ns,
    var,
    var_thread_binding,
  };

  struct object
  { object_type type{}; };

  template <object_type T>
  struct static_object;
}

#include <jank/native_box.hpp>

namespace jank::runtime
{
  using object_ptr = native_box<object>;

  namespace behavior
  {
    template <typename T>
    concept objectable = requires(T * const t)
    {
      { t->equal(std::declval<object const&>()) } -> std::convertible_to<native_bool>;
      { t->to_string() } -> std::convertible_to<native_persistent_string>;
      { t->to_string(std::declval<fmt::memory_buffer&>()) } -> std::same_as<void>;
      { t->to_hash() } -> std::convertible_to<native_integer>;
      { t->base } -> std::same_as<object&>;
    };
  }
}
