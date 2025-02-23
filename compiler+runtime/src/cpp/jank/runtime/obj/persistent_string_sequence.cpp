#include <jank/runtime/obj/persistent_string_sequence.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/core/seq_ext.hpp>
#include <jank/runtime/core/make_box.hpp>

namespace jank::runtime::obj
{
  persistent_string_sequence::persistent_string_sequence(persistent_string_ptr const s)
    : str{ s }
  {
    assert(!s->data.empty());
  }

  persistent_string_sequence::persistent_string_sequence(persistent_string_ptr const s,
                                                         size_t const i)
    : str{ s }
    , index{ i }
  {
    assert(!s->data.empty() && i < s->data.size());
  }

  /* behavior::object_like */
  native_bool persistent_string_sequence::equal(object const &o) const
  {
    return runtime::equal(o, str->data.begin() + index, str->data.end());
  }

  void persistent_string_sequence::to_string(util::string_builder &buff) const
  {
    runtime::to_string(str->data.begin() + index, str->data.end(), "(", ')', buff);
  }

  native_persistent_string persistent_string_sequence::to_string() const
  {
    util::string_builder buff;
    runtime::to_string(str->data.begin() + index, str->data.end(), "(", ')', buff);
    return buff.release();
  }

  native_persistent_string persistent_string_sequence::to_code_string() const
  {
    util::string_builder buff;
    runtime::to_code_string(str->data.begin() + index, str->data.end(), "(", ')', buff);
    return buff.release();
  }

  native_hash persistent_string_sequence::to_hash() const
  {
    return hash::ordered(str->data.begin() + index, str->data.end());
  }

  /* behavior::countable */
  size_t persistent_string_sequence::count() const
  {
    return str->data.size() - index;
  }

  /* behavior::seqable */
  persistent_string_sequence_ptr persistent_string_sequence::seq()
  {
    return this;
  }

  persistent_string_sequence_ptr persistent_string_sequence::fresh_seq() const
  {
    return make_box<persistent_string_sequence>(str, index);
  }

  /* behavior::sequenceable */
  object_ptr persistent_string_sequence::first() const
  {
    return make_box(str->data[index]);
  }

  persistent_string_sequence_ptr persistent_string_sequence::next() const
  {
    auto n(index);
    ++n;

    if(n == str->data.size())
    {
      return nullptr;
    }

    return make_box<persistent_string_sequence>(str, n);
  }

  persistent_string_sequence_ptr persistent_string_sequence::next_in_place()
  {
    ++index;

    if(index == str->data.size())
    {
      return nullptr;
    }

    return this;
  }

  cons_ptr persistent_string_sequence::conj(object_ptr const head)
  {
    return make_box<cons>(head, this);
  }
}
