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

    native_bool equal(object const &) const override;
    native_string to_string() const override;
    void to_string(fmt::memory_buffer &buff) const override;
    native_integer to_hash() const override;

    list const* as_list() const override;
    behavior::seqable const* as_seqable() const override;

    behavior::sequence_ptr seq() const override;
    size_t count() const override;

    behavior::consable const* as_consable() const override;
    native_box<behavior::consable> cons(object_ptr head) const override;

    object_ptr with_meta(object_ptr m) const override;
    behavior::metadatable const* as_metadatable() const override;

    runtime::detail::persistent_list data;
  };
  using list_ptr = native_box<list>;
}
