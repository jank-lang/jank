#include <jank/runtime/obj/native_vector_sequence.hpp>

namespace jank::runtime
{
  obj::native_vector_sequence::static_object(native_vector<object_ptr> const &data, size_t index)
    : data{ data }, index{ index }
  { }
  obj::native_vector_sequence::static_object(native_vector<object_ptr> &&data)
    : data{ std::move(data) }
  { }
  obj::native_vector_sequence::static_object(native_vector<object_ptr> &&data, size_t index)
    : data{ std::move(data) }, index{ index }
  { }

  /* behavior::objectable */
  native_bool obj::native_vector_sequence::equal(object const &o) const
  { return detail::equal(o, data.begin(), data.end()); }

  void obj::native_vector_sequence::to_string(fmt::memory_buffer &buff) const
  { return behavior::detail::to_string(data.begin(), data.end(), '(', ')', buff); }

  native_string obj::native_vector_sequence::to_string() const
  {
    fmt::memory_buffer buff;
    behavior::detail::to_string(data.begin(), data.end(), '(', ')', buff);
    return native_string{ buff.data(), buff.size() };
  }

  native_integer obj::native_vector_sequence::to_hash()
  { return reinterpret_cast<native_integer>(this); }

  /* behavior::seqable */
  obj::native_vector_sequence_ptr obj::native_vector_sequence::seq()
  { return this; }
  obj::native_vector_sequence_ptr obj::native_vector_sequence::fresh_seq()
  { return jank::make_box<obj::native_vector_sequence>(data, index); }

  /* behavior::countable */
  size_t obj::native_vector_sequence::count() const
  { return data.size(); }

  /* behavior::sequence */
  object_ptr obj::native_vector_sequence::first() const
  { return data[index]; }

  obj::native_vector_sequence_ptr obj::native_vector_sequence::next() const
  {
    auto n(index);
    ++n;

    if(n == data.size())
    { return nullptr; }

    return jank::make_box<obj::native_vector_sequence>(data, n);
  }

  obj::native_vector_sequence_ptr obj::native_vector_sequence::next_in_place()
  {
    ++index;

    if(index == data.size())
    { return nullptr; }

    return this;
  }

  object_ptr obj::native_vector_sequence::next_in_place_first()
  {
    ++index;

    if(index == data.size())
    { return nullptr; }

    return data[index];
  }

  obj::cons_ptr obj::native_vector_sequence::cons(object_ptr const head)
  { return make_box<obj::cons>(head, this); }
}
