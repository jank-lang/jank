#include <jank/runtime/obj/native_array_sequence.hpp>

namespace jank::runtime
{
  obj::native_array_sequence::static_object(object_ptr * const arr, size_t const size)
    : arr{ arr }
    , size{ size }
  {
    assert(arr);
    assert(size > 0);
  }

  obj::native_array_sequence::static_object(object_ptr * const arr,
                                            size_t const index,
                                            size_t const size)
    : arr{ arr }
    , index{ index }
    , size{ size }
  {
    assert(arr);
    assert(size > 0);
  }

  /* behavior::objectable */
  native_bool obj::native_array_sequence::equal(object const &o) const
  {
    return detail::equal(o, arr, arr + size);
  }

  void obj::native_array_sequence::to_string(fmt::memory_buffer &buff) const
  {
    return behavior::detail::to_string(arr + index, arr + size, "(", ')', buff);
  }

  native_persistent_string obj::native_array_sequence::to_string() const
  {
    fmt::memory_buffer buff;
    behavior::detail::to_string(arr + index, arr + size, "(", ')', buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_hash obj::native_array_sequence::to_hash() const
  {
    return hash::ordered(arr + index, arr + size);
  }

  /* behavior::seqable */
  obj::native_array_sequence_ptr obj::native_array_sequence::seq()
  {
    return this;
  }

  obj::native_array_sequence_ptr obj::native_array_sequence::fresh_seq()
  {
    return make_box<obj::native_array_sequence>(arr, index, size);
  }

  /* behavior::countable */
  size_t obj::native_array_sequence::count() const
  {
    return size;
  }

  /* behavior::sequence */
  object_ptr obj::native_array_sequence::first() const
  {
    assert(index < size);
    return arr[index];
  }

  obj::native_array_sequence_ptr obj::native_array_sequence::next() const
  {
    auto n(index);
    ++n;

    if(size <= n)
    {
      return nullptr;
    }

    return make_box<obj::native_array_sequence>(arr, n, size);
  }

  obj::native_array_sequence_ptr obj::native_array_sequence::next_in_place()
  {
    ++index;

    if(size <= index)
    {
      return nullptr;
    }

    return this;
  }

  obj::cons_ptr obj::native_array_sequence::conj(object_ptr const head)
  {
    return make_box<obj::cons>(head, this);
  }
}
