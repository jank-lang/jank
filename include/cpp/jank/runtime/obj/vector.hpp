#pragma once

#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/behavior/countable.hpp>
#include <jank/runtime/behavior/consable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/behavior/associatively_readable.hpp>

namespace jank::runtime::obj
{
  struct vector
    :
      virtual object,
      behavior::seqable, behavior::countable, behavior::consable,
      behavior::metadatable,
      behavior::associatively_readable
  {
    static constexpr bool pointer_free{ false };

    vector() = default;
    vector(vector &&) = default;
    vector(vector const &) = default;
    vector(runtime::detail::peristent_vector &&d);
    vector(runtime::detail::peristent_vector const &d);
    template <typename... Args>
    vector(Args &&...args)
      : data{ std::forward<Args>(args)... }
    { }
    ~vector() = default;

    /* TODO: Remove; dupe of ctor. */
    static native_box<vector> create(runtime::detail::peristent_vector const &);
    static native_box<vector> create(behavior::sequence_ptr const &s);

    native_bool equal(object const &) const final;
    native_string to_string() const final;
    void to_string(fmt::memory_buffer &buff) const final;
    native_integer to_hash() const final;

    vector const* as_vector() const final;
    behavior::seqable const* as_seqable() const final;

    behavior::sequence_ptr seq() const final;
    behavior::sequence_ptr fresh_seq() const final;
    size_t count() const final;

    behavior::consable const* as_consable() const final;
    native_box<behavior::consable> cons(object_ptr head) const final;

    object_ptr with_meta(object_ptr m) const final;
    behavior::metadatable const* as_metadatable() const final;

    behavior::associatively_readable const* as_associatively_readable() const final;
    object_ptr get(object_ptr key) const final;
    object_ptr get(object_ptr key, object_ptr fallback) const final;

    runtime::detail::peristent_vector data;
  };
  using vector_ptr = native_box<vector>;
}
