#include <jank/runtime/obj/native_array_sequence.hpp>

namespace jank::runtime
{
  obj::native_array_sequence::static_object(object_ptr * const arr, size_t const size)
    : arr{ arr }, size{ size }
  { }
  obj::native_array_sequence::static_object(object_ptr * const arr, size_t const index, size_t const size)
    : arr{ arr }, index{ index }, size{ size }
  { }

  /* behavior::objectable */
  native_bool obj::native_array_sequence::equal(object const &o) const
  { return detail::equal(o, arr, arr + size); }

  void obj::native_array_sequence::to_string(fmt::memory_buffer &buff) const
  { return behavior::detail::to_string(arr + index, arr + size, '(', ')', buff); }

  native_string obj::native_array_sequence::to_string() const
  {
    fmt::memory_buffer buff;
    behavior::detail::to_string(arr + index, arr + size, '(', ')', buff);
    return native_string{ buff.data(), buff.size() };
  }
  native_integer obj::native_array_sequence::to_hash()
  { return reinterpret_cast<native_integer>(this); }

  /* behavior::seqable */
  obj::native_array_sequence_ptr obj::native_array_sequence::seq()
  { return this; }
  obj::native_array_sequence_ptr obj::native_array_sequence::fresh_seq()
  { return make_box<obj::native_array_sequence>(arr, index, size); }

  /* behavior::countable */
  size_t obj::native_array_sequence::count() const
  { return size; }

  /* behavior::sequence */
  object_ptr obj::native_array_sequence::first() const
  { return arr ? arr[index] : nullptr; }
  obj::native_array_sequence_ptr obj::native_array_sequence::next() const
  {
    auto n(index);
    ++n;

    if(size <= n)
    { return nullptr; }

    return make_box<obj::native_array_sequence>(arr, n, size);
  }
  obj::native_array_sequence_ptr obj::native_array_sequence::next_in_place()
  {
    ++index;

    if(size <= index)
    { return nullptr; }

    return this;
  }
  object_ptr obj::native_array_sequence::next_in_place_first()
  {
    ++index;

    if(size <= index)
    { return nullptr; }

    return arr[index];
  }

  obj::cons_ptr obj::native_array_sequence::cons(object_ptr const head)
  { return make_box<obj::cons>(head, this); }
}
