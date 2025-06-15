#include <jank/runtime/detail/native_persistent_array_map.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::detail
{
  /* TODO: Int sequence to clean this up? */
  static object_ref *make_next_array(object_ref const * const prev,
                                     usize const length,
                                     object_ref const key,
                                     object_ref const value)
  {
    switch(length)
    {
      case 0:
        {
          auto const ret(new(GC) object_ref[2]{ key, value });
          return ret;
        }
      case 2:
        {
          auto const ret(new(GC) object_ref[4]{ prev[0], prev[1], key, value });
          return ret;
        }
      case 4:
        {
          auto const ret(new(GC) object_ref[6]{ prev[0], prev[1], prev[2], prev[3], key, value });
          return ret;
        }
      case 6:
        {
          auto const ret(new(
            GC) object_ref[8]{ prev[0], prev[1], prev[2], prev[3], prev[4], prev[5], key, value });
          return ret;
        }
      case 8:
        {
          auto const ret(new(GC) object_ref[10]{ prev[0],
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
          auto const ret(new(GC) object_ref[12]{ prev[0],
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
          auto const ret(new(GC) object_ref[14]{ prev[0],
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
          auto const ret(new(GC) object_ref[16]{ prev[0],
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
      default:
        throw std::runtime_error{ util::format(
          "Unable to expand array map to size {}. Be sure to check the size prior to insertion and "
          "promote to hash map if needed.",
          (length / 2) + 1) };
    }
  }

  void native_persistent_array_map::insert_unique(object_ref const key, object_ref const val)
  {
    data = make_next_array(data, length, key, val);
    length += 2;
    hash = 0;
  }

  void native_persistent_array_map::insert_or_assign(object_ref const key, object_ref const val)
  {
    if(key->type == runtime::object_type::keyword)
    {
      for(usize i{}; i < length; i += 2)
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
      for(usize i{}; i < length; i += 2)
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

  object_ref native_persistent_array_map::find(object_ref const key) const
  {
    if(key->type == runtime::object_type::keyword)
    {
      for(usize i{}; i < length; i += 2)
      {
        if(data[i] == key)
        {
          return data[i + 1];
        }
      }
    }
    else
    {
      for(usize i{}; i < length; i += 2)
      {
        if(runtime::equal(data[i], key))
        {
          return data[i + 1];
        }
      }
    }
    return {};
  }

  void native_persistent_array_map::erase(object_ref const key)
  {
    if(key->type == runtime::object_type::keyword)
    {
      for(usize i{}; i < length; i += 2)
      {
        if(data[i] == key)
        {
          for(usize k{ i + 2 }; k < length; k += 2)
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
      for(usize i{}; i < length; i += 2)
      {
        if(runtime::equal(data[i], key))
        {
          for(usize k{ i + 2 }; k < length; k += 2)
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

  uhash native_persistent_array_map::to_hash() const
  {
    if(hash != 0)
    {
      return hash;
    }

    return hash = hash::unordered(begin(), end());
  }

  native_persistent_array_map::iterator::iterator(object_ref const * const data, usize const index)
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

  bool native_persistent_array_map::iterator::operator!=(iterator const &rhs) const
  {
    return data != rhs.data || index != rhs.index;
  }

  bool native_persistent_array_map::iterator::operator==(iterator const &rhs) const
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

  usize native_persistent_array_map::size() const
  {
    return length / 2;
  }

  bool native_persistent_array_map::empty() const
  {
    return length == 0;
  }

  native_persistent_array_map native_persistent_array_map::clone() const
  {
    native_persistent_array_map ret{ *this };
    ret.data = new(GC) object_ref[length];
    memcpy(ret.data, data, length * sizeof(object_ref));
    return ret;
  }
}
