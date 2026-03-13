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

    reader_conditional,
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

      case object_type::reader_conditional:
        return "reader_conditional";
    }
    return "unknown";
  }

  enum class object_behavior : u8
  {
    none = 0,
    call = 1 << 0,
    get = 1 << 1,
    find = 1 << 2,
    //associatively_writable,
    //chunkable,
    //collection_like,
    //comparable,
    //conjable,
    //countable,
    //derefable,
    //indexable,
    //map_like,
    //metadatable,
    //nameable,
    //number_like,
    //realizable,
    //ref_like,
    //seqable,
    //sequential,
    //set_like,
    //stackable,
    //transientable
  };

  constexpr object_behavior
  operator&(object_behavior const behaviors, object_behavior const behavior)
  {
    return static_cast<object_behavior>(
      static_cast<std::underlying_type_t<object_behavior>>(behaviors)
      & static_cast<std::underlying_type_t<object_behavior>>(behavior));
  }

  constexpr object_behavior
  operator|(object_behavior const behaviors, object_behavior const behavior)
  {
    return static_cast<object_behavior>(
      static_cast<std::underlying_type_t<object_behavior>>(behaviors)
      | static_cast<std::underlying_type_t<object_behavior>>(behavior));
  }

  using callable_arity_flags = u8;

  using object_ref = oref<struct object>;

  struct object : gc
  {
    object() = delete;
    object(object const &) noexcept;
    object(object &&) noexcept;
    object(object_type, object_behavior) noexcept;
    virtual ~object() = default;

    object &operator=(object const &) noexcept;
    object &operator=(object &&) noexcept;

    /* object_like */
    virtual bool equal(object const &) const;
    virtual jtl::immutable_string to_string() const;
    virtual void to_string(jtl::string_builder &) const;
    virtual jtl::immutable_string to_code_string() const;
    virtual uhash to_hash() const;
    bool has_behavior(object_behavior) const;

    /* behavior::call */
    virtual object_ref call() const;
    virtual object_ref call(object_ref const) const;
    virtual object_ref call(object_ref const, object_ref const) const;
    virtual object_ref call(object_ref const, object_ref const, object_ref const) const;
    virtual object_ref
    call(object_ref const, object_ref const, object_ref const, object_ref const) const;
    virtual object_ref call(object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const) const;
    virtual object_ref call(object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const) const;
    virtual object_ref call(object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const) const;
    virtual object_ref call(object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const) const;
    virtual object_ref call(object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const) const;
    virtual object_ref call(object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const) const;

    /* When dynamically calling a function, we need to know three things:
      *
      * 1. Is the function variadic?
      * 2. Is there an ambiguous fixed overload?
      * 3. How many fixed arguments are required before the packed args?
      *
      * We cannot perform the correct call without all of this information. Since function calls
      * are on the hottest path there is, we pack all of this into a single byte. Questions
      * 1 and 2 each get a bit and question 3 gets 6 bits to store the fixed arg count.
      *
      * From there, when we use it, we strip out the bit for question 2 and we switch/case on
      * the rest. This allows us to do a O(1) jump on the combination of whether it's variadic
      * and the required fixed args. Finally, we only need the question 2 bit to disambiguate
      * one branch of each switch.
      *
      * The ambiguity comes in this case:
      *
      * ```
      * (defn ambiguous
      *   ([a] 1)
      *   ([a & args] args))
      * (ambiguous :a)
      * ```
      *
      * When we call `ambiguous` with a single arg, we want it to match the fixed unary arity.
      * However, given just questions 1 and 3, we will see that we've met the required args
      * and that the function is variadic and we'll instead dispatch to the variadic arity, with
      * an empty sequence for `args`.
      */
    virtual callable_arity_flags get_arity_flags() const;

    /* behavior::get */
    virtual object_ref get(object_ref key) const;
    virtual object_ref get(object_ref key, object_ref fallback) const;
    virtual bool contains(object_ref key) const;

    /* behavior::find */
    virtual object_ref find(object_ref key) const;

    object_type type{};
    object_behavior behaviors{ object_behavior::none };
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
