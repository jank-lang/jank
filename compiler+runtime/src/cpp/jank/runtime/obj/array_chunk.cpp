#include <magic_enum.hpp>

#include <jank/runtime/obj/array_chunk.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/rtti.hpp>

namespace jank::runtime
{
  obj::array_chunk::static_object(native_vector<object_ptr> const &buffer)
    : buffer{ buffer }
  {
  }

  obj::array_chunk::static_object(native_vector<object_ptr> const &buffer, size_t const offset)
    : buffer{ buffer }
    , offset{ offset }
  {
  }

  obj::array_chunk::static_object(native_vector<object_ptr> &&buffer, size_t const offset)
    : buffer{ std::move(buffer) }
    , offset{ offset }
  {
  }

  native_bool obj::array_chunk::equal(object const &o) const
  {
    return &o == &base;
  }

  native_persistent_string obj::array_chunk::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  void obj::array_chunk::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff),
                   "{}@{}",
                   magic_enum::enum_name(base.type),
                   fmt::ptr(&base));
  }

  native_persistent_string obj::array_chunk::to_code_string() const
  {
    return to_string();
  }

  native_hash obj::array_chunk::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  obj::array_chunk_ptr obj::array_chunk::chunk_next() const
  {
    if(offset == buffer.size())
    {
      throw std::runtime_error{ "no more chunk remaining to chunk_next" };
    }
    /* TODO: This copying will be slow. Use a persistent_vector? */
    return make_box<obj::array_chunk>(buffer, offset + 1);
  }

  obj::array_chunk_ptr obj::array_chunk::chunk_next_in_place()
  {
    if(offset == buffer.size())
    {
      throw std::runtime_error{ "no more chunk remaining to chunk_next" };
    }
    ++offset;
    return this;
  }

  size_t obj::array_chunk::count() const
  {
    return buffer.size() - offset;
  }

  object_ptr obj::array_chunk::nth(object_ptr const index) const
  {
    if(index->type == object_type::integer)
    {
      auto const i(static_cast<size_t>(expect_object<obj::integer>(index)->data));
      if(buffer.size() - offset <= i)
      {
        throw std::runtime_error{ fmt::format(
          "out of bounds index {}; array_chunk has a size of {} and offset of {}",
          i,
          buffer.size(),
          offset) };
      }
      return buffer[offset + i];
    }
    else
    {
      throw std::runtime_error{ fmt::format("nth on a array_chunk must be an integer; found {}",
                                            runtime::to_string(index)) };
    }
  }

  object_ptr obj::array_chunk::nth(object_ptr const index, object_ptr const fallback) const
  {
    if(index->type == object_type::integer)
    {
      auto const i(static_cast<size_t>(expect_object<obj::integer>(index)->data));
      if(buffer.size() - offset <= i)
      {
        throw std::runtime_error{ fmt::format(
          "out of bounds index {}; array_chunk has a size of {} and offset of {}",
          i,
          buffer.size(),
          offset) };
      }
      return buffer[offset + i];
    }
    else
    {
      return fallback;
    }
  }
}
