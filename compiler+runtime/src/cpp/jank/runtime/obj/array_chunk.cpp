#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/obj/array_chunk.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/rtti.hpp>

namespace jank::runtime::obj
{
  array_chunk::array_chunk(native_vector<object_ptr> const &buffer)
    : buffer{ buffer }
  {
  }

  array_chunk::array_chunk(native_vector<object_ptr> const &buffer, size_t const offset)
    : buffer{ buffer }
    , offset{ offset }
  {
  }

  array_chunk::array_chunk(native_vector<object_ptr> &&buffer, size_t const offset)
    : buffer{ std::move(buffer) }
    , offset{ offset }
  {
  }

  native_bool array_chunk::equal(object const &o) const
  {
    return &o == &base;
  }

  native_persistent_string array_chunk::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void array_chunk::to_string(util::string_builder &buff) const
  {
    fmt::format_to(std::back_inserter(buff), "{}@{}", object_type_str(base.type), fmt::ptr(&base));
  }

  native_persistent_string array_chunk::to_code_string() const
  {
    return to_string();
  }

  native_hash array_chunk::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  array_chunk_ptr array_chunk::chunk_next() const
  {
    if(offset == buffer.size())
    {
      throw std::runtime_error{ "no more chunk remaining to chunk_next" };
    }
    /* TODO: This copying will be slow. Use a persistent_vector? */
    return make_box<array_chunk>(buffer, offset + 1);
  }

  array_chunk_ptr array_chunk::chunk_next_in_place()
  {
    if(offset == buffer.size())
    {
      throw std::runtime_error{ "no more chunk remaining to chunk_next" };
    }
    ++offset;
    return this;
  }

  size_t array_chunk::count() const
  {
    return buffer.size() - offset;
  }

  object_ptr array_chunk::nth(object_ptr const index) const
  {
    if(index->type == object_type::integer)
    {
      auto const i(static_cast<size_t>(expect_object<integer>(index)->data));
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

  object_ptr array_chunk::nth(object_ptr const index, object_ptr const fallback) const
  {
    if(index->type == object_type::integer)
    {
      auto const i(static_cast<size_t>(expect_object<integer>(index)->data));
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
