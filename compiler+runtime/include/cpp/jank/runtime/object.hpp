#pragma once

#include <concepts>

#include <jank/type.hpp>
#include <jank/util/string_builder.hpp>

namespace jank::runtime
{
  enum class object_type : uint8_t
  {
    nil,

    boolean,
    integer,
    real,
    ratio,

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
    integer_range,
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

    native_pointer_wrapper,

    atom,
    volatile_,
    reduced,
    delay,
    future,
    ns,

    var,
    var_thread_binding,
    var_unbound_root,

    tagged_literal,
  };

  constexpr char const *object_type_str(object_type const type)
  {
    switch(type)
    {
      case object_type::nil:
        return "nil";

      case object_type::boolean:
        return "boolean";
      case object_type::integer:
        return "integer";
      case object_type::real:
        return "real";
      case object_type::ratio:
        return "ratio";

      case object_type::persistent_string:
        return "persistent_string";
      case object_type::persistent_string_sequence:
        return "persistent_string_sequence";

      case object_type::keyword:
        return "keyword";
      case object_type::symbol:
        return "symbol";
      case object_type::character:
        return "character";

      case object_type::persistent_list:
        return "persistent_list";
      case object_type::persistent_list_sequence:
        return "persistent_list_sequence";

      case object_type::persistent_vector:
        return "persistent_vector";
      case object_type::transient_vector:
        return "transient_vector";
      case object_type::persistent_vector_sequence:
        return "persistent_vector_sequence";

      case object_type::persistent_array_map:
        return "persistent_array_map";
      case object_type::persistent_array_map_sequence:
        return "persistent_array_map_sequence";

      case object_type::persistent_hash_map:
        return "persistent_hash_map";
      case object_type::transient_hash_map:
        return "transient_hash_map";
      case object_type::persistent_hash_map_sequence:
        return "persistent_hash_map_sequence";

      case object_type::persistent_sorted_map:
        return "persistent_sorted_map";
      case object_type::transient_sorted_map:
        return "transient_sorted_map";
      case object_type::persistent_sorted_map_sequence:
        return "persistent_sorted_map_sequence";

      case object_type::persistent_hash_set:
        return "persistent_hash_set";
      case object_type::transient_hash_set:
        return "transient_hash_set";
      case object_type::persistent_hash_set_sequence:
        return "persistent_hash_set_sequence";

      case object_type::persistent_sorted_set:
        return "persistent_sorted_set";
      case object_type::transient_sorted_set:
        return "transient_sorted_set";
      case object_type::persistent_sorted_set_sequence:
        return "persistent_sorted_set_sequence";

      case object_type::cons:
        return "cons";
      case object_type::lazy_sequence:
        return "lazy_sequence";
      case object_type::range:
        return "range";
      case object_type::integer_range:
        return "integer_range";
      case object_type::repeat:
        return "repeat";
      case object_type::iterator:
        return "iterator";
      case object_type::native_array_sequence:
        return "native_array_sequence";
      case object_type::native_vector_sequence:
        return "native_vector_sequence";

      case object_type::chunk_buffer:
        return "chunk_buffer";
      case object_type::array_chunk:
        return "array_chunk";
      case object_type::chunked_cons:
        return "chunked_cons";

      case object_type::native_function_wrapper:
        return "native_function_wrapper";
      case object_type::jit_function:
        return "jit_function";
      case object_type::jit_closure:
        return "jit_closure";
      case object_type::multi_function:
        return "multi_function";

      case object_type::native_pointer_wrapper:
        return "native_pointer_wrapper";

      case object_type::atom:
        return "atom";
      case object_type::volatile_:
        return "volatile_";
      case object_type::reduced:
        return "reduced";
      case object_type::delay:
        return "delay";
      case object_type::future:
        return "future";
      case object_type::ns:
        return "ns";

      case object_type::var:
        return "var";
      case object_type::var_thread_binding:
        return "var_thread_binding";
      case object_type::var_unbound_root:
        return "var_unbound_root";

      case object_type::tagged_literal:
        return "tagged_literal";
    }
    return "unknown";
  }

  struct object
  {
    object_type type{};
  };
}

namespace jank::runtime::behavior
{
  /* Every object implements this behavior and it's the only behavior in common with
   * every object. If there's any other behavior which every object must have, it should
   * instead just be a part of this. */
  template <typename T>
  concept object_like = requires(T * const t) {
    { T::obj_type } -> std::convertible_to<object_type>;

    /* Determines is the specified object is equal, but not necessarily identical, to
     * the current object. Identical means having the same address, the same identity.
     * Equal just means having equal values. Equivalent means having equal values of the
     * same type. :O Here, we're just focused on equality. */
    { t->equal(std::declval<object const &>()) } -> std::convertible_to<native_bool>;

    /* Returns a string version of the object, generally for printing or displaying. This
     * is distinct from its code representation, which doesn't yet have a corresponding
     * function in this behavior. */
    { t->to_string() } -> std::convertible_to<native_persistent_string>;
    { t->to_string(std::declval<util::string_builder &>()) } -> std::same_as<void>;

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
     * type to shift the object pointer and cast it into the fully typed object. */
    { t->base } -> std::same_as<object &>;
  };
}

#include <jank/runtime/native_box.hpp>

namespace jank::runtime
{
  using object_ptr = native_box<object>;

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
