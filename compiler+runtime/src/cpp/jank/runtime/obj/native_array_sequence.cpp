#include <jank/runtime/obj/native_array_sequence.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/seq_ext.hpp>

namespace jank::runtime::obj
{
  native_array_sequence::native_array_sequence(object_ref * const arr, usize const size)
    : arr{ arr }
    , size{ size }
  {
    jank_debug_assert(arr);
    jank_debug_assert(size > 0);
  }

  native_array_sequence::native_array_sequence(object_ref * const arr,
                                               usize const index,
                                               usize const size)
    : arr{ arr }
    , index{ index }
    , size{ size }
  {
    jank_debug_assert(arr);
    jank_debug_assert(size > 0);
  }

  /* behavior::object_like */
  bool native_array_sequence::equal(object const &o) const
  {
    return runtime::equal(o, arr + index, arr + size);
  }

  void native_array_sequence::to_string(util::string_builder &buff) const
  {
    runtime::to_string(arr + index, arr + size, "(", ')', buff);
  }

  jtl::immutable_string native_array_sequence::to_string() const
  {
    util::string_builder buff;
    runtime::to_string(arr + index, arr + size, "(", ')', buff);
    return buff.release();
  }

  jtl::immutable_string native_array_sequence::to_code_string() const
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
  native_array_sequence_ref native_array_sequence::seq()
  {
    return this;
  }

  native_array_sequence_ref native_array_sequence::fresh_seq()
  {
    return make_box<native_array_sequence>(arr, index, size);
  }

  /* behavior::countable */
  usize native_array_sequence::count() const
  {
    return size - index;
  }

  /* behavior::sequence */
  object_ref native_array_sequence::first() const
  {
    jank_debug_assert(index < size);
    return arr[index];
  }

  native_array_sequence_ref native_array_sequence::next() const
  {
    auto n(index);
    ++n;

    if(size <= n)
    {
      return {};
    }

    return make_box<native_array_sequence>(arr, n, size);
  }

  native_array_sequence_ref native_array_sequence::next_in_place()
  {
    ++index;

    if(size <= index)
    {
      return {};
    }

    return this;
  }

  cons_ref native_array_sequence::conj(object_ref const head)
  {
    return make_box<cons>(head, this);
  }
}
