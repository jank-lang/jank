#include <jank/runtime/obj/array_chunk.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  array_chunk::array_chunk(native_vector<object_ref> const &buffer)
    : buffer{ buffer }
  {
  }

  array_chunk::array_chunk(native_vector<object_ref> const &buffer, usize const offset)
    : buffer{ buffer }
    , offset{ offset }
  {
  }

  array_chunk::array_chunk(native_vector<object_ref> &&buffer, usize const offset)
    : buffer{ std::move(buffer) }
    , offset{ offset }
  {
  }

  bool array_chunk::equal(object const &o) const
  {
    return &o == &base;
  }

  jtl::immutable_string array_chunk::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void array_chunk::to_string(util::string_builder &buff) const
  {
    util::format_to(buff, "{}@{}", object_type_str(base.type), &base);
  }

  jtl::immutable_string array_chunk::to_code_string() const
  {
    return to_string();
  }

  uhash array_chunk::to_hash() const
  {
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
  }

  array_chunk_ref array_chunk::chunk_next() const
  {
    if(offset == buffer.size())
    {
      throw std::runtime_error{ "no more chunk remaining to chunk_next" };
    }
    /* TODO: This copying will be slow. Use a persistent_vector? */
    return make_box<array_chunk>(buffer, offset + 1);
  }

  array_chunk_ref array_chunk::chunk_next_in_place()
  {
    if(offset == buffer.size())
    {
      throw std::runtime_error{ "no more chunk remaining to chunk_next" };
    }
    ++offset;
    return this;
  }

  usize array_chunk::count() const
  {
    return buffer.size() - offset;
  }

  object_ref array_chunk::nth(object_ref const index) const
  {
    if(index->type == object_type::integer)
    {
      auto const i(expect_object<integer>(index)->data);
      if(i < 0 || buffer.size() - offset <= static_cast<size_t>(i))
      {
        throw std::runtime_error{ util::format(
          "out of bounds index {}; array_chunk has a size of {} and offset of {}",
          i,
          buffer.size(),
          offset) };
      }
      return buffer[offset + i];
    }
    else
    {
      throw std::runtime_error{ util::format("nth on a array_chunk must be an integer; found {}",
                                             runtime::to_string(index)) };
    }
  }

  object_ref array_chunk::nth(object_ref const index, object_ref const fallback) const
  {
    if(index->type == object_type::integer)
    {
      auto const i(expect_object<integer>(index)->data);
      if(i < 0 || buffer.size() - offset <= static_cast<size_t>(i))
      {
        return fallback;
      }
      return buffer[offset + i];
    }
    else
    {
      return fallback;
    }
  }
}
