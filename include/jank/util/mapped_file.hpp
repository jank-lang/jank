#pragma once

#include <jank/option.hpp>

namespace jank::util
{
  struct mapped_file
  {
    mapped_file() = default;
    ~mapped_file();

    int fd{};
    char const *head{};
    size_t size{};
  };

  option<mapped_file> map_file(char const * const file);
}
