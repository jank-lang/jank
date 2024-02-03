#include <jank/runtime/obj/native_vector_sequence.hpp>

namespace jank::runtime
{
  obj::native_vector_sequence::static_object(native_vector<object_ptr> const &data, size_t index)
    : data{ data }
    , index{ index }
  {
    assert(!this->data.empty());
  }

  obj::native_vector_sequence::static_object(native_vector<object_ptr> &&data)
    : data{ std::move(data) }
  {
    assert(!this->data.empty());
  }

  obj::native_vector_sequence::static_object(native_vector<object_ptr> &&data, size_t index)
    : data{ std::move(data) }
    , index{ index }
  {
    assert(!this->data.empty());
  }

  /* behavior::objectable */
  native_bool obj::native_vector_sequence::equal(object const &o) const
  {
    return detail::equal(o, data.begin(), data.end());
  }

  void obj::native_vector_sequence::to_string(fmt::memory_buffer &buff) const
  {
    return behavior::detail::to_string(data.begin(), data.end(), "(", ')', buff);
  }

  native_persistent_string obj::native_vector_sequence::to_string() const
  {
    fmt::memory_buffer buff;
    behavior::detail::to_string(data.begin(), data.end(), "(", ')', buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_hash obj::native_vector_sequence::to_hash()
  {
    return hash::ordered(data.begin(), data.end());
  }

  /* behavior::seqable */
  obj::native_vector_sequence_ptr obj::native_vector_sequence::seq()
  {
    return data.empty() ? nullptr : this;
  }

  obj::native_vector_sequence_ptr obj::native_vector_sequence::fresh_seq()
  {
    return data.empty() ? nullptr : make_box<obj::native_vector_sequence>(data, index);
  }

  /* behavior::countable */
  size_t obj::native_vector_sequence::count() const
  {
    return data.size();
  }

  /* behavior::sequence */
  object_ptr obj::native_vector_sequence::first() const
  {
    assert(index < data.size());
    return data[index];
  }

  obj::native_vector_sequence_ptr obj::native_vector_sequence::next() const
  {
    auto n(index);
    ++n;

    if(n == data.size())
    {
      return nullptr;
    }

    return make_box<obj::native_vector_sequence>(data, n);
  }

  obj::native_vector_sequence_ptr obj::native_vector_sequence::next_in_place()
  {
    ++index;

    if(index == data.size())
    {
      return nullptr;
    }

    return this;
  }

  object_ptr obj::native_vector_sequence::next_in_place_first()
  {
    ++index;

    if(index == data.size())
    {
      return nullptr;
    }

    return data[index];
  }

  obj::cons_ptr obj::native_vector_sequence::cons(object_ptr const head)
  {
    return make_box<obj::cons>(head, data.empty() ? nullptr : this);
  }
}
