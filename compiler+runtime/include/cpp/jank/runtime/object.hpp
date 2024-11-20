#pragma once

#include <concepts>

#include <fmt/format.h>

#include <jank/type.hpp>

/* TODO: Move to obj namespace */
namespace jank::runtime
{
  enum class object_type
  {
    nil,

    boolean,
    integer,
    real,

    persistent_string,
    persistent_string_sequence,

    keyword,
    symbol,
    character,

    persistent_list,
    persistent_list_sequence,

    persistent_vector,
    transient_vector,
    persistent_vector_sequence,

    persistent_array_map,
    /* TODO: transient_array_map */
    persistent_array_map_sequence,

    persistent_hash_map,
    transient_hash_map,
    persistent_hash_map_sequence,

    persistent_sorted_map,
    transient_sorted_map,
    persistent_sorted_map_sequence,

    persistent_hash_set,
    transient_hash_set,
    persistent_hash_set_sequence,

    persistent_sorted_set,
    transient_sorted_set,
    persistent_sorted_set_sequence,

    cons,
    lazy_sequence,
    range,
    repeat,
    iterator,
    native_array_sequence,
    native_vector_sequence,

    chunk_buffer,
    array_chunk,
    chunked_cons,

    native_function_wrapper,
    jit_function,
    jit_closure,
    multi_function,

    atom,
    volatile_,
    reduced,
    delay,
    ns,

    var,
    var_thread_binding,
    var_unbound_root,
  };

  struct object
  {
    object_type type{};
  };

  template <object_type T>
  struct static_object;
}

#include <jank/runtime/native_box.hpp>

namespace jank::runtime
{
  using object_ptr = native_box<object>;

  namespace behavior
  {
    /* Every object implements this behavior and it's the only behavior in common with
     * every object. If there's any other behavior which every object must have, it should
     * instead just be a part of this. */
    template <typename T>
    concept object_like = requires(T * const t) {
      /* Determines is the specified object is equal, but not necessarily identical, to
       * the current object. Identical means having the same address, the same identity.
       * Equal just means having equal values. Equivalent means having equal values of the
       * same type. :O Here, we're just focused on equality. */
      { t->equal(std::declval<object const &>()) } -> std::convertible_to<native_bool>;

      /* Returns a string version of the object, generally for printing or displaying. This
       * is distinct from its code representation, which doesn't yet have a corresponding
       * function in this behavior. */
      { t->to_string() } -> std::convertible_to<native_persistent_string>;
      { t->to_string(std::declval<fmt::memory_buffer &>()) } -> std::same_as<void>;

      /* Returns the code representation of the object. */
      { t->to_code_string() } -> std::convertible_to<native_persistent_string>;

      /* Returns a deterministic hash value for the object. For some objects, like functions
       * and transients, the hash is actually just the object's address. For others, it's
       * based on the value, or values, within the object. There are a set of hash functions
       * which should be used for this in hash.hpp. */
      { t->to_hash() } -> std::convertible_to<native_integer>;

      /* Every object needs to have this base field, which is the actual object field.
       * When we pass around object pointers, we pass around pointers to this field within
       * the overall object. This field stores the type of the object and we use that
       * type to shift the object pointer and cast it into the fully typed static_object. */
      { t->base } -> std::same_as<object &>;
    };
  }

  /* This isn't a great name, but it represents more than just value equality, since it
   * also includes type equality. Otherwise, [] equals '(). This is important when deduping
   * constants during codegen, since we don't want to be lossy in how we generate values. */
  struct very_equal_to
  {
    bool operator()(object_ptr const lhs, object_ptr const rhs) const noexcept;
  };
}

namespace std
{
  template <>
  struct hash<jank::runtime::object_ptr>
  {
    size_t operator()(jank::runtime::object_ptr const o) const noexcept;
  };

  template <>
  struct hash<jank::runtime::object>
  {
    size_t operator()(jank::runtime::object const &o) const noexcept;
  };

  template <>
  struct equal_to<jank::runtime::object_ptr>
  {
    bool operator()(jank::runtime::object_ptr const lhs,
                    jank::runtime::object_ptr const rhs) const noexcept;
  };
}
