#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <filesystem>

#include <jank/util/mapped_file.hpp>
#include <jank/option.hpp>

namespace jank::util
{
  mapped_file::mapped_file(mapped_file &&mf)
    : fd{ mf.fd }, head{ mf.head }, size{ mf.size }
  {
    mf.fd = -1;
    mf.head = nullptr;
  }

  mapped_file::mapped_file(int const f, char const * const h, size_t const s)
    : fd{ f }, head{ h }, size{ s }
  { }

  mapped_file::~mapped_file()
  {
    if(head != nullptr)
    { munmap(reinterpret_cast<void*>(const_cast<char*>(head)), size); }
    if(fd >= 0)
    { ::close(fd); }
  }

  result<mapped_file, std::string> map_file(char const * const file)
  {
    if(!std::filesystem::exists(file))
    { return err("file doesn't exist"); }
    auto const file_size(std::filesystem::file_size(file));
    auto const fd(::open(file, O_RDONLY));
    if(fd < 0)
    { return err("unable to open file"); }
    auto const * const head(reinterpret_cast<char const*>(mmap(0, file_size, PROT_READ, MAP_PRIVATE, fd, 0)));
    if(head == MAP_FAILED)
    { return err("unable to map file"); }
    return ok(jank::util::mapped_file{fd, head, file_size});
  }
}
