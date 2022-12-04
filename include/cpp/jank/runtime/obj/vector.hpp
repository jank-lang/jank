#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/behavior/countable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::obj
{
  struct vector : object, behavior::seqable, behavior::countable, pool_item_base<vector>, behavior::metadatable
  {
    vector() = default;
    vector(vector &&) = default;
    vector(vector const &) = default;
    vector(runtime::detail::vector_type &&d);
    vector(runtime::detail::vector_type const &d);
    template <typename... Args>
    vector(Args &&...args)
      : data{ std::forward<Args>(args)... }
    { }
    ~vector() = default;

    static runtime::detail::box_type<vector> create(runtime::detail::vector_type const &);
    template <typename... Args>
    static runtime::detail::box_type<vector> create(Args &&...args)
    { return make_box<vector>(std::forward<Args>(args)...); }

    runtime::detail::boolean_type equal(object const &) const override;
    runtime::detail::string_type to_string() const override;
    runtime::detail::integer_type to_hash() const override;

    vector const* as_vector() const override;
    behavior::seqable const* as_seqable() const override;

    behavior::sequence_ptr seq() const override;
    size_t count() const override;

    object_ptr with_meta(object_ptr const &m) const override;
    behavior::metadatable const* as_metadatable() const override;

    runtime::detail::vector_type data;
  };
  using vector_ptr = runtime::detail::box_type<vector>;
}
