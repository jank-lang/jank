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

    native_bool equal(object const &) const final;
    native_string to_string() const final;
    void to_string(fmt::memory_buffer &buff) const final;
    native_integer to_hash() const final;

    string const* as_string() const final;

    size_t count() const final;

    object_ptr with_meta(object_ptr m) const final;
    behavior::metadatable const* as_metadatable() const final;

    native_string data;
  };
  using string_ptr = native_box<string>;
}
