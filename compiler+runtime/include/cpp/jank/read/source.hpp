#pragma once

#include <jank/runtime/object.hpp>
#include <jank/native_persistent_string.hpp>

namespace jank::read
{
  constexpr auto no_source_path{ "NO_SOURCE_PATH" };

  struct source_position
  {
    static source_position const unknown;

    native_bool operator==(source_position const &rhs) const;
    native_bool operator!=(source_position const &rhs) const;

    size_t offset{}, line{ 1 }, col{ 1 };
  };

  struct source
  {
    static source const unknown;

    source() = delete;
    source(source const &) = default;
    source(source &&) noexcept = default;
    source(source_position const &start);
    source(source_position const &start, source_position const &end);
    source(native_persistent_string const &file_path,
           source_position const &start,
           source_position const &end);
    source(native_persistent_string const &file_path,
           source_position const &start,
           source_position const &end,
           runtime::object_ptr macro_expansion);

    native_bool operator==(source const &rhs) const;
    native_bool operator!=(source const &rhs) const;

    native_persistent_string file_path;
    /* Note that start may be equal to end, if the source occupies a single byte. */
    source_position start, end;
    runtime::object_ptr macro_expansion{};
  };

  std::ostream &operator<<(std::ostream &os, source_position const &p);
  std::ostream &operator<<(std::ostream &os, source const &s);
}
