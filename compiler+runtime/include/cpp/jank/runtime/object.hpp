#pragma once

#include <concepts>

#include <jank/type.hpp>
#include <jtl/string_builder.hpp>

namespace jank::runtime
{
  enum class object_type : u8
  {
    nil,

    boolean,
    integer,
    big_integer,
    big_decimal,
    real,
    ratio,

    persistent_string,
    persistent_string_sequence,

    keyword,
    symbol,
    character,

    persistent_list,

    persistent_vector,
    transient_vector,
    persistent_vector_sequence,

    persistent_array_map,
    transient_array_map,
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
    deferred_cpp_function,
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

    re_pattern,
    re_matcher,
    uuid,
    inst,

    opaque_box,
  };

  [[gnu::visibility("default")]]
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
      case object_type::big_integer:
        return "big_integer";
      case object_type::big_decimal:
        return "big_decimal";
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

      case object_type::persistent_vector:
        return "persistent_vector";
      case object_type::transient_vector:
        return "transient_vector";
      case object_type::persistent_vector_sequence:
        return "persistent_vector_sequence";

      case object_type::persistent_array_map:
        return "persistent_array_map";
      case object_type::transient_array_map:
        return "transient_array_map";
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
      case object_type::deferred_cpp_function:
        return "deferred_cpp_function";
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

      case object_type::re_pattern:
        return "re_pattern";
      case object_type::re_matcher:
        return "re_matcher";
      case object_type::uuid:
        return "uuid";
      case object_type::inst:
        return "inst";

      case object_type::opaque_box:
        return "opaque_box";
    }
    return "unknown";
  }

  struct object : gc
  {
    object() = default;
    object(object const &) noexcept;
    object(object &&) noexcept;
    object(object_type) noexcept;
    virtual ~object() = default;

    object &operator=(object const &) noexcept;
    object &operator=(object &&) noexcept;

    virtual bool equal(object const &) const;
    virtual jtl::immutable_string to_string() const;
    virtual void to_string(jtl::string_builder &) const;
    virtual jtl::immutable_string to_code_string() const;
    virtual uhash to_hash() const;

    object_type type{};
  };

  namespace obj
  {
    struct nil;
  }
}

namespace jank::runtime::behavior
{
  /* Every object implements this behavior and it's the only behavior in common with
   * every object. If there's any other behavior which every object must have, it should
   * instead just be a part of this. */
  template <typename T>
  concept object_like = requires(T * const t) {
    { T::obj_type } -> std::convertible_to<object_type>;

    std::is_base_of_v<object, T>;
  };
}

#include <jank/runtime/oref.hpp>

namespace jank::runtime
{
  using object_ref = oref<object>;

  /* This isn't a great name, but it represents more than just value equality, since it
   * also includes type equality. Otherwise, [] equals '(). This is important when deduping
   * constants during codegen, since we don't want to be lossy in how we generate values. */
  struct very_equal_to
  {
    bool operator()(object_ref const lhs, object_ref const rhs) const noexcept;
  };

  bool operator==(object const *, object_ref const);
  bool operator!=(object const *, object_ref const);
}

namespace std
{
  template <>
  struct hash<jank::runtime::object_ref>
  {
    size_t operator()(jank::runtime::object_ref const o) const noexcept;
  };

  template <>
  struct hash<jank::runtime::object>
  {
    size_t operator()(jank::runtime::object const &o) const noexcept;
  };

  template <>
  struct equal_to<jank::runtime::object_ref>
  {
    bool operator()(jank::runtime::object_ref const lhs,
                    jank::runtime::object_ref const rhs) const noexcept;
  };
}
