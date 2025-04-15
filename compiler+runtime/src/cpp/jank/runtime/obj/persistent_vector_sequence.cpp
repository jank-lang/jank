#include <jank/runtime/obj/persistent_vector_sequence.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/seq_ext.hpp>

namespace jank::runtime::obj
{
  persistent_vector_sequence::persistent_vector_sequence(persistent_vector_ref const v)
    : vec{ v }
  {
    jank_debug_assert(!v->data.empty());
  }

  persistent_vector_sequence::persistent_vector_sequence(persistent_vector_ref const v,
                                                         size_t const i)
    : vec{ v }
    , index{ i }
  {
    jank_debug_assert(index < v->data.size());
    jank_debug_assert(0 < v->data.size() - index);
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

  jtl::immutable_string persistent_vector_sequence::to_string() const
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

  jtl::immutable_string persistent_vector_sequence::to_code_string() const
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
  persistent_vector_sequence_ref persistent_vector_sequence::seq()
  {
    return this;
  }

  persistent_vector_sequence_ref persistent_vector_sequence::fresh_seq() const
  {
    return make_box<persistent_vector_sequence>(vec, index);
  }

  /* behavior::sequenceable */
  object_ref persistent_vector_sequence::first() const
  {
    return vec->data[index];
  }

  persistent_vector_sequence_ref persistent_vector_sequence::next() const
  {
    auto n(index);
    ++n;

    if(n == vec->data.size())
    {
      return {};
    }

    return make_box<persistent_vector_sequence>(vec, n);
  }

  persistent_vector_sequence_ref persistent_vector_sequence::next_in_place()
  {
    ++index;

    if(index == vec->data.size())
    {
      return {};
    }

    return this;
  }

  cons_ref persistent_vector_sequence::conj(object_ref const head)
  {
    return make_box<cons>(head, this);
  }
}
