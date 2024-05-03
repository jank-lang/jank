#include <jank/runtime/obj/chunk_buffer.hpp>

namespace jank::runtime
{
  obj::chunk_buffer::static_object(size_t const capacity)
    : capacity{ capacity }
  {
    buffer.reserve(capacity);
  }

  obj::chunk_buffer::static_object(object_ptr const capacity)
  {
    auto const c(to_int(capacity));
    if(c < 0)
    {
      throw std::runtime_error{ fmt::format("invalid capacity: {}", c) };
    }
    this->capacity = c;
    buffer.reserve(c);
  }

  native_bool obj::chunk_buffer::equal(object const &o) const
  {
    return &o == &base;
  }

  native_persistent_string obj::chunk_buffer::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  void obj::chunk_buffer::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff),
                   "{}@{}",
                   magic_enum::enum_name(base.type),
                   fmt::ptr(&base));
  }

  native_hash obj::chunk_buffer::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  size_t obj::chunk_buffer::count() const
  {
    return buffer.size();
  }

  void obj::chunk_buffer::append(object_ptr const o)
  {
    if(buffer.size() == capacity)
    {
      throw std::runtime_error{ "chunk buffer is already full" };
    }
    buffer.emplace_back(o);
  }

  obj::array_chunk_ptr obj::chunk_buffer::chunk()
  {
    auto const ret(make_box<obj::array_chunk>(std::move(buffer)));
    buffer.clear();
    buffer.shrink_to_fit();
    capacity = 0;
    return ret;
  }
}
