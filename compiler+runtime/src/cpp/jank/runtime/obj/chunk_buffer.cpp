#include <jank/runtime/obj/chunk_buffer.hpp>
#include <jank/runtime/obj/array_chunk.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  chunk_buffer::chunk_buffer(usize const capacity)
    : capacity{ capacity }
  {
    buffer.reserve(capacity);
  }

  chunk_buffer::chunk_buffer(object_ref const capacity)
  {
    auto const c(to_int(capacity));
    if(c < 0)
    {
      throw std::runtime_error{ util::format("invalid capacity: {}", c) };
    }
    this->capacity = c;
    buffer.reserve(c);
  }

  native_bool chunk_buffer::equal(object const &o) const
  {
    return &o == &base;
  }

  jtl::immutable_string chunk_buffer::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void chunk_buffer::to_string(util::string_builder &buff) const
  {
    util::format_to(buff, "{}@{}", object_type_str(base.type), &base);
  }

  jtl::immutable_string chunk_buffer::to_code_string() const
  {
    return to_string();
  }

  native_hash chunk_buffer::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  usize chunk_buffer::count() const
  {
    return buffer.size();
  }

  void chunk_buffer::append(object_ref const o)
  {
    if(buffer.size() == capacity)
    {
      throw std::runtime_error{ "chunk buffer is already full" };
    }
    buffer.emplace_back(o);
  }

  array_chunk_ref chunk_buffer::chunk()
  {
    auto const ret(make_box<array_chunk>(std::move(buffer)));
    buffer.clear();
    buffer.shrink_to_fit();
    capacity = 0;
    return ret;
  }
}
