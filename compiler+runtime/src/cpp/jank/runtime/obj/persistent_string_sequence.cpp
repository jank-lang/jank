#include <jank/runtime/obj/persistent_string_sequence.hpp>

namespace jank::runtime
{
  obj::persistent_string_sequence::static_object(obj::persistent_string_ptr const s)
    : str{ s }
  {
    assert(!s->data.empty());
  }

  obj::persistent_string_sequence::static_object(obj::persistent_string_ptr const s, size_t const i)
    : str{ s }
    , index{ i }
  {
    assert(!s->data.empty() && i < s->data.size());
  }

  /* behavior::objectable */
  native_bool obj::persistent_string_sequence::equal(object const &o) const
  {
    return detail::equal(o, str->data.begin(), str->data.end());
  }

  void obj::persistent_string_sequence::to_string(fmt::memory_buffer &buff) const
  {
    return behavior::detail::to_string(str->data.begin() + index, str->data.end(), "(", ')', buff);
  }

  native_persistent_string obj::persistent_string_sequence::to_string() const
  {
    fmt::memory_buffer buff;
    behavior::detail::to_string(str->data.begin() + index, str->data.end(), "(", ')', buff);
    return { buff.data(), buff.size() };
  }

  native_hash obj::persistent_string_sequence::to_hash() const
  {
    return hash::ordered(str->data.begin(), str->data.end());
  }

  /* behavior::countable */
  size_t obj::persistent_string_sequence::count() const
  {
    return str->data.size() - index;
  }

  /* behavior::seqable */
  obj::persistent_string_sequence_ptr obj::persistent_string_sequence::seq()
  {
    return this;
  }

  obj::persistent_string_sequence_ptr obj::persistent_string_sequence::fresh_seq() const
  {
    return make_box<obj::persistent_string_sequence>(str, index);
  }

  /* behavior::sequenceable */
  object_ptr obj::persistent_string_sequence::first() const
  {
    return make_box(str->data[index]);
  }

  obj::persistent_string_sequence_ptr obj::persistent_string_sequence::next() const
  {
    auto n(index);
    ++n;

    if(n == str->data.size())
    {
      return nullptr;
    }

    return jank::make_box<obj::persistent_string_sequence>(str, n);
  }

  obj::persistent_string_sequence_ptr obj::persistent_string_sequence::next_in_place()
  {
    ++index;

    if(index == str->data.size())
    {
      return nullptr;
    }

    return this;
  }

  obj::cons_ptr obj::persistent_string_sequence::conj(object_ptr const head)
  {
    return make_box<obj::cons>(head, this);
  }
}
