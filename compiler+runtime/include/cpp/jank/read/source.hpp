#pragma once

#include <jank/native_persistent_string.hpp>

namespace jank::read
{
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

    native_bool operator==(source const &rhs) const;
    native_bool operator!=(source const &rhs) const;

    native_persistent_string file_path;
    /* Note that start may be equal to end, if the source occupies a single byte. */
    source_position start, end;
  };

  std::ostream &operator<<(std::ostream &os, source_position const &p);
  std::ostream &operator<<(std::ostream &os, source const &s);
}