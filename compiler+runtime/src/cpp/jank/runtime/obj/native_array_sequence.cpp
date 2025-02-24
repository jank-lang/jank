#include <jank/runtime/obj/native_array_sequence.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/seq_ext.hpp>

namespace jank::runtime::obj
{
  native_array_sequence::native_array_sequence(object_ptr * const arr, size_t const size)
    : arr{ arr }
    , size{ size }
  {
    assert(arr);
    assert(size > 0);
  }

  native_array_sequence::native_array_sequence(object_ptr * const arr,
                                               size_t const index,
                                               size_t const size)
    : arr{ arr }
    , index{ index }
    , size{ size }
  {
    assert(arr);
    assert(size > 0);
  }

  /* behavior::object_like */
  native_bool native_array_sequence::equal(object const &o) const
  {
    return runtime::equal(o, arr + index, arr + size);
  }

  void native_array_sequence::to_string(util::string_builder &buff) const
  {
    runtime::to_string(arr + index, arr + size, "(", ')', buff);
  }

  native_persistent_string native_array_sequence::to_string() const
  {
    util::string_builder buff;
    runtime::to_string(arr + index, arr + size, "(", ')', buff);
    return buff.release();
  }

  native_persistent_string native_array_sequence::to_code_string() const
  {
    util::string_builder buff;
    runtime::to_code_string(arr + index, arr + size, "(", ')', buff);
    return buff.release();
  }

  native_hash native_array_sequence::to_hash() const
  {
    return hash::ordered(arr + index, arr + size);
  }

  /* behavior::seqable */
  native_array_sequence_ptr native_array_sequence::seq()
  {
    return this;
  }

  native_array_sequence_ptr native_array_sequence::fresh_seq()
  {
    return make_box<native_array_sequence>(arr, index, size);
  }

  /* behavior::countable */
  size_t native_array_sequence::count() const
  {
    return size - index;
  }

  /* behavior::sequence */
  object_ptr native_array_sequence::first() const
  {
    assert(index < size);
    return arr[index];
  }

  native_array_sequence_ptr native_array_sequence::next() const
  {
    auto n(index);
    ++n;

    if(size <= n)
    {
      return nullptr;
    }

    return make_box<native_array_sequence>(arr, n, size);
  }

  native_array_sequence_ptr native_array_sequence::next_in_place()
  {
    ++index;

    if(size <= index)
    {
      return nullptr;
    }

    return this;
  }

  cons_ptr native_array_sequence::conj(object_ptr const head)
  {
    return make_box<cons>(head, this);
  }
}
