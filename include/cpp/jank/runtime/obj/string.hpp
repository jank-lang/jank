#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/countable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::obj
{
  /* TODO: Seqable. */
  struct string : object, behavior::countable, behavior::metadatable
  {
    string() = default;
    string(string &&) = default;
    string(string const &) = default;
    string(runtime::detail::string_type const &d);
    string(runtime::detail::string_type &&d);
    ~string() = default;

    runtime::detail::boolean_type equal(object const &) const override;
    runtime::detail::string_type to_string() const override;
    void to_string(fmt::memory_buffer &buff) const override;
    runtime::detail::integer_type to_hash() const override;

    string const* as_string() const override;

    size_t count() const override;

    object_ptr with_meta(object_ptr m) const override;
    behavior::metadatable const* as_metadatable() const override;

    runtime::detail::string_type data;
  };
  using string_ptr = runtime::detail::box_type<string>;
}
