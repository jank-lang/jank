#include <jank/runtime/obj/persistent_string_sequence.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/core/seq_ext.hpp>
#include <jank/runtime/core/make_box.hpp>

namespace jank::runtime::obj
{
  persistent_string_sequence::persistent_string_sequence(persistent_string_ref const s)
    : str{ s }
  {
    jank_debug_assert(!s->data.empty());
  }

  persistent_string_sequence::persistent_string_sequence(persistent_string_ref const s,
                                                         size_t const i)
    : str{ s }
    , index{ i }
  {
    jank_debug_assert(!s->data.empty() && i < s->data.size());
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

  jtl::immutable_string persistent_string_sequence::to_string() const
  {
    util::string_builder buff;
    runtime::to_string(str->data.begin() + index, str->data.end(), "(", ')', buff);
    return buff.release();
  }

  jtl::immutable_string persistent_string_sequence::to_code_string() const
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
  persistent_string_sequence_ref persistent_string_sequence::seq()
  {
    return this;
  }

  persistent_string_sequence_ref persistent_string_sequence::fresh_seq() const
  {
    return make_box<persistent_string_sequence>(str, index);
  }

  /* behavior::sequenceable */
  object_ref persistent_string_sequence::first() const
  {
    return make_box(str->data[index]);
  }

  persistent_string_sequence_ref persistent_string_sequence::next() const
  {
    auto n(index);
    ++n;

    if(n == str->data.size())
    {
      return {};
    }

    return make_box<persistent_string_sequence>(str, n);
  }

  persistent_string_sequence_ref persistent_string_sequence::next_in_place()
  {
    ++index;

    if(index == str->data.size())
    {
      return {};
    }

    return this;
  }

  cons_ref persistent_string_sequence::conj(object_ref const head)
  {
    return make_box<cons>(head, this);
  }
}
