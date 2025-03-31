#pragma once

#include <jank/runtime/object.hpp>
#include <jtl/immutable_string.hpp>

namespace jank::read
{
  constexpr auto no_source_path{ "NO_SOURCE_PATH" };

  struct source_position
  {
    static source_position const unknown;

    native_bool operator==(source_position const &rhs) const;
    native_bool operator!=(source_position const &rhs) const;
    native_bool operator<=(source_position const &rhs) const;
    native_bool operator>=(source_position const &rhs) const;

    jtl::immutable_string to_string() const;

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
    source(jtl::immutable_string const &file_path,
           source_position const &start,
           source_position const &end);
    source(jtl::immutable_string const &file_path,
           source_position const &start,
           source_position const &end,
           runtime::object_ptr macro_expansion);

    source &operator=(source const &rhs) = default;
    source &operator=(source &&rhs) = default;

    native_bool operator==(source const &rhs) const;
    native_bool operator!=(source const &rhs) const;

    native_bool overlaps(source const &) const;

    jtl::immutable_string to_string() const;

    jtl::immutable_string file_path;
    /* Note that start may be equal to end, if the source occupies a single byte. */
    source_position start, end;
    /* The form (and its meta) from which the form at this location expanded. Note
     * that this isn't stored within the jank/source key. It has its own key, since
     * it may be attached to synthetic data which has no source info. */
    runtime::object_ptr macro_expansion{};
  };

  std::ostream &operator<<(std::ostream &os, source_position const &p);
  std::ostream &operator<<(std::ostream &os, source const &s);
}
