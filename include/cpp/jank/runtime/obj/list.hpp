#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/behavior/countable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::obj
{
  struct list : object, behavior::seqable, behavior::countable, behavior::metadatable
  {
    list() = default;
    list(list &&) = default;
    list(list const &) = default;
    list(runtime::detail::list_type &&d);
    list(runtime::detail::list_type const &d);
    template <typename... Args>
    list(Args &&...args)
      : data{ std::forward<Args>(args)... }
    { }
    ~list() = default;

    static runtime::detail::box_type<list> create(runtime::detail::list_type const &l);
    static runtime::detail::box_type<list> create(behavior::sequence_ptr const &s);
    template <typename... Args>
    static runtime::detail::box_type<list> create(std::in_place_t, Args &&...args)
    { return make_box<list>(std::forward<Args>(args)...); }

    runtime::detail::boolean_type equal(object const &) const override;
    runtime::detail::string_type to_string() const override;
    void to_string(fmt::memory_buffer &buff) const override;
    runtime::detail::integer_type to_hash() const override;

    list const* as_list() const override;
    behavior::seqable const* as_seqable() const override;

    behavior::sequence_ptr seq() const override;
    size_t count() const override;

    object_ptr with_meta(object_ptr m) const override;
    behavior::metadatable const* as_metadatable() const override;

    runtime::detail::list_type data;
  };
  using list_ptr = list*;
}
