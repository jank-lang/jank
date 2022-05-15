#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <filesystem>

#include <jank/util/mapped_file.hpp>
#include <jank/option.hpp>

namespace jank::util
{
  mapped_file::~mapped_file()
  {
    if(head != nullptr)
    { munmap(reinterpret_cast<void*>(const_cast<char*>(head)), size); }
    if(fd >= 0)
    { ::close(fd); }
  }

  option<mapped_file> map_file(char const * const file)
  {
    if(!std::filesystem::exists(file))
    {
      std::cerr << "File doesn't exist: " << file << "\n";
      return none;
    }
    auto const file_size(std::filesystem::file_size(file));
    auto const fd(::open(file, O_RDONLY));
    if(fd < 0)
    {
      std::cerr << "Unable to open file: " << file << "\n";
      return none;
    }
    auto const *head(reinterpret_cast<char const*>(mmap(0, file_size, PROT_READ, MAP_PRIVATE, fd, 0)));
    if(head == MAP_FAILED)
    {
      std::cerr << "Unable to map file: " << file << "\n";
      return none;
    }
    return some(jank::util::mapped_file{fd, head, file_size});
  }
}
