#pragma once

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
    string(native_string const &d);
    string(native_string &&d);
    ~string() = default;

    native_bool equal(object const &) const override;
    native_string to_string() const override;
    void to_string(fmt::memory_buffer &buff) const override;
    native_integer to_hash() const override;

    string const* as_string() const override;

    size_t count() const override;

    object_ptr with_meta(object_ptr m) const override;
    behavior::metadatable const* as_metadatable() const override;

    native_string data;
  };
  using string_ptr = native_box<string>;
}
