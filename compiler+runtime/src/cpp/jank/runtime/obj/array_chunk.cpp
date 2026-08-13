#include <jank/runtime/obj/array_chunk.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  array_chunk::array_chunk()
    : object{ obj_type, obj_behaviors }
  {
  }

  array_chunk::array_chunk(native_vector<object_ref> const &buffer)
    : object{ obj_type, obj_behaviors }
    , buffer{ buffer }
  {
  }

  array_chunk::array_chunk(native_vector<object_ref> const &buffer, usize const offset)
    : object{ obj_type, obj_behaviors }
    , buffer{ buffer }
    , offset{ offset }
  {
  }

  array_chunk::array_chunk(native_vector<object_ref> &&buffer, usize const offset)
    : object{ obj_type, obj_behaviors }
    , buffer{ std::move(buffer) }
    , offset{ offset }
  {
  }

  array_chunk_ref array_chunk::chunk_next() const
  {
    if(offset == buffer.size())
    {
      throw std::runtime_error{ "There are no more chunks remaining for `chunk-next`." };
    }
    /* TODO: This copying will be slow. Use a persistent_vector? */
    return make_box<array_chunk>(buffer, offset + 1);
  }

  array_chunk_ref array_chunk::chunk_next_in_place()
  {
    if(offset == buffer.size())
    {
      throw std::runtime_error{ "There are no more chunks remaining for `chunk-next`." };
    }
    ++offset;
    return runtime::detail::untagged(this);
  }

  usize array_chunk::count() const
  {
    return buffer.size() - offset;
  }

  object_ref array_chunk::nth(object_ref const index) const
  {
    if(is_integral(index))
    {
      auto const i(to_i64(index));
      if(i < 0 || buffer.size() - offset <= static_cast<size_t>(i))
      {
        throw std::runtime_error{ util::format(
          "The index `{}` is out of bounds for this `array_chunk`.",
          i) };
      }
      return buffer[offset + i];
    }
    else
    {
      throw std::runtime_error{ util::format(
        "The `nth` operation on an `array_chunk` requires an integer index, not a `{}`.",
        object_type_str(index.get_type())) };
    }
  }

  object_ref array_chunk::nth(object_ref const index, object_ref const fallback) const
  {
    if(is_integral(index))
    {
      auto const i(to_i64(index));
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
