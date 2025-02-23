#include <jank/runtime/obj/persistent_vector_sequence.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/seq_ext.hpp>

namespace jank::runtime::obj
{
  persistent_vector_sequence::persistent_vector_sequence(persistent_vector_ptr const v)
    : vec{ v }
  {
    assert(!v->data.empty());
  }

  persistent_vector_sequence::persistent_vector_sequence(persistent_vector_ptr const v,
                                                         size_t const i)
    : vec{ v }
    , index{ i }
  {
    assert(index < v->data.size());
    assert(0 < v->data.size() - index);
  }

  /* behavior::object_like */
  native_bool persistent_vector_sequence::equal(object const &o) const
  {
    return runtime::equal(
      o,
      vec->data.begin() + static_cast<decltype(persistent_vector::data)::difference_type>(index),
      vec->data.end());
  }

  void persistent_vector_sequence::to_string(util::string_builder &buff) const
  {
    runtime::to_string(vec->data.begin()
                         + static_cast<decltype(persistent_vector::data)::difference_type>(index),
                       vec->data.end(),
                       "(",
                       ')',
                       buff);
  }

  native_persistent_string persistent_vector_sequence::to_string() const
  {
    util::string_builder buff;
    runtime::to_string(vec->data.begin()
                         + static_cast<decltype(persistent_vector::data)::difference_type>(index),
                       vec->data.end(),
                       "(",
                       ')',
                       buff);
    return buff.release();
  }

  native_persistent_string persistent_vector_sequence::to_code_string() const
  {
    util::string_builder buff;
    runtime::to_code_string(
      vec->data.begin() + static_cast<decltype(persistent_vector::data)::difference_type>(index),
      vec->data.end(),
      "(",
      ')',
      buff);
    return buff.release();
  }

  native_hash persistent_vector_sequence::to_hash() const
  {
    return hash::ordered(vec->data.begin()
                           + static_cast<decltype(persistent_vector::data)::difference_type>(index),
                         vec->data.end());
  }

  /* behavior::countable */
  size_t persistent_vector_sequence::count() const
  {
    return vec->data.size() - index;
  }

  /* behavior::seqable */
  persistent_vector_sequence_ptr persistent_vector_sequence::seq()
  {
    return this;
  }

  persistent_vector_sequence_ptr persistent_vector_sequence::fresh_seq() const
  {
    return make_box<persistent_vector_sequence>(vec, index);
  }

  /* behavior::sequenceable */
  object_ptr persistent_vector_sequence::first() const
  {
    return vec->data[index];
  }

  persistent_vector_sequence_ptr persistent_vector_sequence::next() const
  {
    auto n(index);
    ++n;

    if(n == vec->data.size())
    {
      return nullptr;
    }

    return make_box<persistent_vector_sequence>(vec, n);
  }

  persistent_vector_sequence_ptr persistent_vector_sequence::next_in_place()
  {
    ++index;

    if(index == vec->data.size())
    {
      return nullptr;
    }

    return this;
  }

  cons_ptr persistent_vector_sequence::conj(object_ptr const head)
  {
    return make_box<cons>(head, this);
  }
}
