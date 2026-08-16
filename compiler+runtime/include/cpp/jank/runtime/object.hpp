#pragma once

#include <concepts>

#include <jank/type.hpp>
#include <jtl/string_builder.hpp>

namespace jank::runtime
{
  namespace obj
  {
    struct persistent_list;
    struct keyword;
  }

  enum class object_type : u8
  {
    nil,

    boolean,
    integer,
    small_integer,
    big_integer,
    big_decimal,
    real,
    small_real,
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
    jit_variadic_function,
    jit_closure,
    jit_variadic_closure,
    deferred_cpp_function,
    multi_function,

    native_pointer_wrapper,

    atom,
    volatile_,
    reduced,
    delay,
    future,
    promise,
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

    array,

    reader_conditional,

    exception_info,

    max = exception_info,
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
      case object_type::small_integer:
        return "small_integer";
      case object_type::big_integer:
        return "big_integer";
      case object_type::big_decimal:
        return "big_decimal";
      case object_type::real:
        return "real";
      case object_type::small_real:
        return "small_real";
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
      case object_type::jit_variadic_function:
        return "jit_variadic_function";
      case object_type::jit_closure:
        return "jit_closure";
      case object_type::jit_variadic_closure:
        return "jit_variadic_closure";
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
      case object_type::promise:
        return "promise";
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

      case object_type::array:
        return "array";

      case object_type::reader_conditional:
        return "reader_conditional";

      case object_type::exception_info:
        return "exception_info";
    }
    return "unknown";
  }

  enum class array_element_type : u8
  {
    boolean = static_cast<u8>(object_type::max) + 1,
    character,
    u8,
    i8,
    u16,
    i16,
    u32,
    i32,
    u64,
    i64,
    f32,
    f64,
    object
  };

  constexpr char const *element_type_str(array_element_type const type)
  {
    switch(type)
    {
      case array_element_type::boolean:
        return "bool";
      case array_element_type::character:
        return "char";
      case array_element_type::u8:
        return "u8";
      case array_element_type::i8:
        return "i8";
      case array_element_type::u16:
        return "u16";
      case array_element_type::i16:
        return "i16";
      case array_element_type::u32:
        return "u32";
      case array_element_type::i32:
        return "i32";
      case array_element_type::u64:
        return "u64";
      case array_element_type::i64:
        return "i64";
      case array_element_type::f32:
        return "f32";
      case array_element_type::f64:
        return "f64";
      case array_element_type::object:
        return "object";
    }

    return "unknown";
  }


  /* The old, closed object model relied on each object having an `object_type`. We would then
   * visit each object type as a means of dynamic dispatch. This worked well for a closed object
   * model, but Clojure's object model is inherently open. Using lots of inheritance, as Clojure
   * does, is prohibitively slow in C++, so we kept the closed object model for a couple of years
   * while I spent hammock time to consider alternatives.
   *
   * This `object_behavior` approach is such an alternative. Instead of having an enum for each
   * object type, we have an enum for each object behavior. This means we have a closed set of
   * builtin behaviors, but an open set of object types. Each behavior maps to a bit in a bit set
   * and each object has a bit set which describes its behaviors. The behaviors map to virtual
   * calls on the `object` type. Every object has those virtual fns, they generally just throw
   * by default, saying the object doesn't support that behavior. The way to know if a behavior
   * is supported is to check the corresponding behavior bit.
   *
   * Right now, objects have both an `object_type` and a bit set for object behaviors. This will
   * be the case until we port all previous concept-based behaviors into virtual functions within
   * `object` and a corresponding behavior in this enum. When doing so, we also need to update all
   * `visit` uses for that concept behavior with a bit check and virtual call instead. We need to
   * be mindful to benchmark as we go, to ensure that the new virtual behaviors are comparable to
   * the visit-style behavior. If we can't get comparable performance for a behavior, we should
   * leave it as a `visit` style behavior for now. */
  enum class object_behavior : u16
  {
    none = 0,
    call = 1 << 0,
    get = 1 << 1,
    find = 1 << 2,
    compare = 1 << 3,
    number_like = 1 << 4,
    ref_like = 1 << 5,
    seqable = 1 << 6,
    sequence_like = 1 << 7,
    sequence_like_in_place = 1 << 8,
    indexable = 1 << 9,
    array_like = 1 << 10,
    //associatively_writable,
    //chunkable,
    //collection_like,
    //conjable,
    //countable,
    //derefable,
    //map_like,
    //metadatable,
    //nameable,
    //realizable,
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
    virtual object_ref call(object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const,
                            object_ref const) const;

    /* behavior::get */
    virtual object_ref get(object_ref key) const;
    virtual object_ref get(object_ref key, object_ref fallback) const;
    virtual bool contains(object_ref key) const;

    /* behavior::find */
    virtual object_ref find(object_ref key) const;

    /* behavior::indexable
     *
     * Indexable is meant to provide efficient item access in a
     * collection, given an index. Alas, Clojure implements it
     * for sequences in O(n) as well.
     *
     * In jank, the `runtime::nth` functions will handle the O(n)
     * use cases and this behavior is reserved for more efficient
     * usage. */

    /* Given an index, return the item at that index or throw. */
    virtual object_ref nth(object_ref index) const;

    /* Given an index, return the item at that index or return the fallback. */
    virtual object_ref nth(object_ref index, object_ref fallback) const;

    /* behavior::compare */
    /* Returns how this object compares to the specified object. Comparison, unlike equality,
     * can only be done for objects of the same type. If there's a type mismatch, this function
     * is expected to throw. There are three cases to handle:
     *
     * 1. Comparison should return less than 0 if this object is less than the param
     * 2. Comparison should return 0 if the objects are equal
     * 3. Comparison should return greater than 0 if this object is greater than the param
     *
     * For sequences, all values need to be considered for comparison.
     */
    virtual i64 compare(object const &) const;

    /* behavior::ref_like */
    virtual void set_validator(object_ref const vf);
    virtual object_ref get_validator() const;
    virtual void add_watch(object_ref const key, object_ref const fn);
    virtual void remove_watch(object_ref const key);

    /* behavior::number_like */
    virtual i64 to_integer() const;
    virtual f64 to_real() const;

    /* behavior::seqable */
    /* Returns a (potentially shared) seq, which could just be `this`, if we're already a
     * seq. However, must return a nullptr for empty seqs. Returning a non-null pointer to
     * an empty seq is UB. */
    virtual object_ref seq() const;

    /* Returns a unique seq which can be updated in place. This is an optimization which allows
     * one allocation for a fresh seq which can then be mutated any number of times to traverse
     * the data. Also must return nullptr when the sequence is empty. */
    virtual object_ref fresh_seq() const;

    /* behavior::sequence_like */
    virtual object_ref first() const;

    /* Steps the sequence forward and returns nullptr if there is no more remaining
     * or a pointer to the remaining sequence.
     *
     * Next must always return a fresh seq. */
    virtual object_ref next() const;

    /* Appends the specified object to the beginning of the current sequence. However, if
     * the current sequence is empty, it must create a cons onto nullptr. It's invalid to
     * have a cons onto an empty sequence. */
    //virtual object_ref conj() const;

    /* behavior::sequence_like_in_place */
    /* Each call to next() allocates a new sequence, since it's polymorphic. When iterating
     * over a large sequence, this can mean a _lot_ of allocations. However, if you own the
     * sequence you have, typically meaning it wasn't a parameter, then you can mutate it
     * in place using this function. No allocations will happen.
     *
     * If you don't own your sequence, you can call next() on it once, to get one you
     * do own, and then next_in_place() on that to your heart's content.
     *
     * Using an object after calling next_in_place() on it is UB, even if the return value
     * is the same object. Its ownership has transferred to the return of next_in_place().
     * Don't do this:
     *
     *   (let [s (fresh-seq ...)
     *         s'  (-> s next-in-place)
     *         s'' (-> s next-in-place next-in-place)]
     *                 ^---- UB!! s' owns seq
     *     ...)
     *
     * Do this instead:
     *
     *   (let [s (fresh-seq ...)
     *         s'  (-> s next-in-place)
     *         s'' (-> s' next-in-place next-in-place)]
     *                 ^---- OK: seq ownership transferred from s' to s''
     *     ...)
     *
     * This ownership transfer enables next_in_place() optimizations where the input
     * sequence_like_in_place can sometimes be left in an inconsistent state. For example, if returning
     * nullptr, the ownership of the input sequence_like_in_place has been transferred to nullptr.
     * The input sequence_like_in_place is thus made unreachable. This assumption can be used
     * to elide certain cleanup code. This also applies if (a carefully considered!) allocation
     * is made to return a new sequence_like_in_place, making the input sequence_like_in_place unreachable.
     *
     * next_in_place() can also assume the sequence_like_in_place is non-empty,
     * having retained any and all invariants from being returned from {fresh_}seq() or next{_in_place}().
     * This enables some checks at the beginning of the member function to be elided when
     * compared to next(), such as bounds or emptiness checks. */
    virtual object_ref next_in_place();

    virtual array_element_type get_element_type() const;
    virtual object_ref aset(i64 const index, object_ref const val);
    virtual object_ref aclone() const;

    object_type type{};
    object_behavior behaviors{ object_behavior::none };
  };

  namespace obj
  {
    struct nil;
  }
}

#ifndef JANK_JUST_OBJECT_PLEASE

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

  /* This is the same as `very_equal_to`, but it also compares the meta of each value.
   * This is important for lifting constants for IR and codegen, since values with different
   * meta need to be treated differently so that the meta can carry over into codegen. */
  struct very_equal_to_with_meta
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

#endif
