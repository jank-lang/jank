#include <jank/runtime/obj/native_vector_sequence.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/seq_ext.hpp>

namespace jank::runtime::obj
{
  native_vector_sequence::native_vector_sequence(native_vector<object_ref> const &data,
                                                 usize index)
    : data{ data }
    , index{ index }
  {
    jank_debug_assert(!this->data.empty());
  }

  native_vector_sequence::native_vector_sequence(native_vector<object_ref> &&data)
    : data{ std::move(data) }
  {
    jank_debug_assert(!this->data.empty());
  }

  native_vector_sequence::native_vector_sequence(native_vector<object_ref> &&data, usize index)
    : data{ std::move(data) }
    , index{ index }
  {
    jank_debug_assert(!this->data.empty());
  }

  native_vector_sequence::native_vector_sequence(jtl::option<object_ref> const &meta,
                                                 native_vector<object_ref> &&data)
    : data{ std::move(data) }
    , meta{ meta }
  {
  }

  /* behavior::object_like */
  bool native_vector_sequence::equal(object const &o) const
  {
    return runtime::equal(o, data.begin(), data.end());
  }

  void native_vector_sequence::to_string(util::string_builder &buff) const
  {
    runtime::to_string(data.begin(), data.end(), "(", ')', buff);
  }

  jtl::immutable_string native_vector_sequence::to_string() const
  {
    util::string_builder buff;
    runtime::to_string(data.begin(), data.end(), "(", ')', buff);
    return buff.release();
  }

  jtl::immutable_string native_vector_sequence::to_code_string() const
  {
    util::string_builder buff;
    runtime::to_code_string(data.begin(), data.end(), "(", ')', buff);
    return buff.release();
  }

  uhash native_vector_sequence::to_hash()
  {
    return hash::ordered(data.begin(), data.end());
  }

  /* behavior::seqable */
  native_vector_sequence_ref native_vector_sequence::seq()
  {
    return data.empty() ? native_vector_sequence_ref{} : this;
  }

  native_vector_sequence_ref native_vector_sequence::fresh_seq() const
  {
    return data.empty() ? native_vector_sequence_ref{} : make_box<native_vector_sequence>(data, index);
  }

  /* behavior::countable */
  usize native_vector_sequence::count() const
  {
    return data.size() - index;
  }

  /* behavior::sequence */
  object_ref native_vector_sequence::first() const
  {
    jank_debug_assert(index < data.size());
    return data[index];
  }

  native_vector_sequence_ref native_vector_sequence::next() const
  {
    auto n(index);
    ++n;

    if(n == data.size())
    {
      return {};
    }

    return make_box<native_vector_sequence>(data, n);
  }

  native_vector_sequence_ref native_vector_sequence::next_in_place()
  {
    ++index;

    if(index == data.size())
    {
      return {};
    }

    return this;
  }

  cons_ref native_vector_sequence::conj(object_ref const head)
  {
    return make_box<cons>(head, data.empty() ? nullptr : this);
  }

  native_vector_sequence_ref native_vector_sequence::with_meta(object_ref const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }

}
