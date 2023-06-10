#pragma once

#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/behavior/countable.hpp>
#include <jank/runtime/behavior/consable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::obj
{
  struct list
    :
      virtual object,
      behavior::seqable, behavior::countable, behavior::consable,
      behavior::metadatable
  {
    static constexpr bool pointer_free{ false };

    list() = default;
    list(list &&) = default;
    list(list const &) = default;
    list(runtime::detail::persistent_list &&d);
    list(runtime::detail::persistent_list const &d);
    template <typename... Args>
    list(Args &&...args)
      : data{ std::forward<Args>(args)... }
    { }
    ~list() = default;

    static native_box<list> create(behavior::sequence_ptr const &s);

    native_bool equal(object const &) const final;
    native_string to_string() const final;
    void to_string(fmt::memory_buffer &buff) const final;
    native_integer to_hash() const final;

    list const* as_list() const final;
    behavior::seqable const* as_seqable() const final;

    behavior::sequence_ptr seq() const final;
    behavior::sequence_ptr fresh_seq() const final;
    size_t count() const final;

    behavior::consable const* as_consable() const final;
    native_box<behavior::consable> cons(object_ptr head) const final;

    object_ptr with_meta(object_ptr m) const final;
    behavior::metadatable const* as_metadatable() const final;

    runtime::detail::persistent_list data;
  };
  using list_ptr = native_box<list>;
}
