#include <jank/runtime/obj/chunk_buffer.hpp>
#include <jank/runtime/obj/array_chunk.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  chunk_buffer::chunk_buffer()
    : object{ obj_type, obj_behaviors }
  {
  }

  chunk_buffer::chunk_buffer(usize const capacity)
    : object{ obj_type, obj_behaviors }
    , capacity{ capacity }
  {
    buffer.reserve(capacity);
  }

  chunk_buffer::chunk_buffer(object_ref const capacity)
    : object{ obj_type, obj_behaviors }
  {
    auto const c(to_int(capacity));
    if(c < 0)
    {
      throw std::runtime_error{ util::format("invalid capacity: {}", c) };
    }
    this->capacity = c;
    buffer.reserve(c);
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
