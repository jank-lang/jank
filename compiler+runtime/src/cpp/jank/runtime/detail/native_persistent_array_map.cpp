#include <jank/runtime/detail/native_persistent_array_map.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::detail
{
  /* TODO: Int sequence to clean this up? */
  static object_ptr *make_next_array(object_ptr const * const prev,
                                     size_t const length,
                                     object_ptr const key,
                                     object_ptr const value)
  {
    switch(length)
    {
      case 0:
        {
          auto const ret(new(GC) object_ptr[2]{ key, value });
          return ret;
        }
      case 2:
        {
          auto const ret(new(GC) object_ptr[4]{ prev[0], prev[1], key, value });
          return ret;
        }
      case 4:
        {
          auto const ret(new(GC) object_ptr[6]{ prev[0], prev[1], prev[2], prev[3], key, value });
          return ret;
        }
      case 6:
        {
          auto const ret(new(
            GC) object_ptr[8]{ prev[0], prev[1], prev[2], prev[3], prev[4], prev[5], key, value });
          return ret;
        }
      case 8:
        {
          auto const ret(new(GC) object_ptr[10]{ prev[0],
                                                 prev[1],
                                                 prev[2],
                                                 prev[3],
                                                 prev[4],
                                                 prev[5],
                                                 prev[6],
                                                 prev[7],
                                                 key,
                                                 value });
          return ret;
        }
      case 10:
        {
          auto const ret(new(GC) object_ptr[12]{ prev[0],
                                                 prev[1],
                                                 prev[2],
                                                 prev[3],
                                                 prev[4],
                                                 prev[5],
                                                 prev[6],
                                                 prev[7],
                                                 prev[8],
                                                 prev[9],
                                                 key,
                                                 value });
          return ret;
        }
      case 12:
        {
          auto const ret(new(GC) object_ptr[14]{ prev[0],
                                                 prev[1],
                                                 prev[2],
                                                 prev[3],
                                                 prev[4],
                                                 prev[5],
                                                 prev[6],
                                                 prev[7],
                                                 prev[8],
                                                 prev[9],
                                                 prev[10],
                                                 prev[11],
                                                 key,
                                                 value });
          return ret;
        }
      case 14:
        {
          auto const ret(new(GC) object_ptr[16]{ prev[0],
                                                 prev[1],
                                                 prev[2],
                                                 prev[3],
                                                 prev[4],
                                                 prev[5],
                                                 prev[6],
                                                 prev[7],
                                                 prev[8],
                                                 prev[9],
                                                 prev[10],
                                                 prev[11],
                                                 prev[12],
                                                 prev[13],
                                                 key,
                                                 value });
          return ret;
        }
      // TODO: Convert to hash map.
      default:
        throw std::runtime_error{ util::format("unsupported array size: {}", length + 2) };
    }
  }

  void native_persistent_array_map::insert_unique(object_ptr const key, object_ptr const val)
  {
    data = make_next_array(data, length, key, val);
    length += 2;
    hash = 0;
  }

  void native_persistent_array_map::insert_or_assign(object_ptr const key, object_ptr const val)
  {
    if(key->type == runtime::object_type::keyword)
    {
      for(size_t i{}; i < length; i += 2)
      {
        if(data[i] == key)
        {
          data[i + 1] = val;
          hash = 0;
          return;
        }
      }
    }
    else
    {
      for(size_t i{}; i < length; i += 2)
      {
        if(runtime::equal(data[i], key))
        {
          data[i + 1] = val;
          hash = 0;
          return;
        }
      }
    }
    insert_unique(key, val);
  }

  object_ptr native_persistent_array_map::find(object_ptr const key) const
  {
    if(key->type == runtime::object_type::keyword)
    {
      for(size_t i{}; i < length; i += 2)
      {
        if(data[i] == key)
        {
          return data[i + 1];
        }
      }
    }
    else
    {
      for(size_t i{}; i < length; i += 2)
      {
        if(runtime::equal(data[i], key))
        {
          return data[i + 1];
        }
      }
    }
    return nullptr;
  }

  void native_persistent_array_map::erase(object_ptr const key)
  {
    if(key->type == runtime::object_type::keyword)
    {
      for(size_t i{}; i < length; i += 2)
      {
        if(data[i] == key)
        {
          for(size_t k{ i + 2 }; k < length; k += 2)
          {
            data[k - 2] = data[k];
            data[k - 1] = data[k + 1];
          }

          length -= 2;
          hash = 0;
          return;
        }
      }
    }
    else
    {
      for(size_t i{}; i < length; i += 2)
      {
        if(runtime::equal(data[i], key))
        {
          for(size_t k{ i + 2 }; k < length; k += 2)
          {
            data[k - 2] = data[k];
            data[k - 1] = data[k + 1];
          }

          length -= 2;
          hash = 0;
          return;
        }
      }
    }
  }

  native_hash native_persistent_array_map::to_hash() const
  {
    if(hash != 0)
    {
      return hash;
    }

    return hash = hash::unordered(begin(), end());
  }

  native_persistent_array_map::iterator::iterator(object_ptr const * const data, size_t const index)
    : data{ data }
    , index{ index }
  {
  }

  native_persistent_array_map::iterator::value_type
  native_persistent_array_map::iterator::operator*() const
  {
    return { data[index], data[index + 1] };
  }

  native_persistent_array_map::iterator &native_persistent_array_map::iterator::operator++()
  {
    index += 2;
    return *this;
  }

  native_bool native_persistent_array_map::iterator::operator!=(iterator const &rhs) const
  {
    return data != rhs.data || index != rhs.index;
  }

  native_bool native_persistent_array_map::iterator::operator==(iterator const &rhs) const
  {
    return !(*this != rhs);
  }

  native_persistent_array_map::iterator &
  native_persistent_array_map::iterator::operator=(native_persistent_array_map::iterator const &rhs)
  {
    if(this == &rhs)
    {
      return *this;
    }

    data = rhs.data;
    index = rhs.index;
    return *this;
  }

  native_persistent_array_map::const_iterator native_persistent_array_map::begin() const
  {
    return const_iterator{ data, 0 };
  }

  native_persistent_array_map::const_iterator native_persistent_array_map::end() const
  {
    return const_iterator{ data, length };
  }

  size_t native_persistent_array_map::size() const
  {
    return length / 2;
  }

  native_bool native_persistent_array_map::empty() const
  {
    return length == 0;
  }

  native_persistent_array_map native_persistent_array_map::clone() const
  {
    native_persistent_array_map ret{ *this };
    ret.data = new(GC) object_ptr[length];
    memcpy(ret.data, data, length * sizeof(object_ptr));
    return ret;
  }
}
