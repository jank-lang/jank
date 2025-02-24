#include <jank/runtime/obj/native_vector_sequence.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/seq_ext.hpp>

namespace jank::runtime::obj
{
  native_vector_sequence::native_vector_sequence(native_vector<object_ptr> const &data,
                                                 size_t index)
    : data{ data }
    , index{ index }
  {
    assert(!this->data.empty());
  }

  native_vector_sequence::native_vector_sequence(native_vector<object_ptr> &&data)
    : data{ std::move(data) }
  {
    assert(!this->data.empty());
  }

  native_vector_sequence::native_vector_sequence(native_vector<object_ptr> &&data, size_t index)
    : data{ std::move(data) }
    , index{ index }
  {
    assert(!this->data.empty());
  }

  native_vector_sequence::native_vector_sequence(option<object_ptr> const &meta,
                                                 native_vector<object_ptr> &&data)
    : data{ std::move(data) }
    , meta{ meta }
  {
  }

  /* behavior::object_like */
  native_bool native_vector_sequence::equal(object const &o) const
  {
    return runtime::equal(o, data.begin(), data.end());
  }

  void native_vector_sequence::to_string(util::string_builder &buff) const
  {
    runtime::to_string(data.begin(), data.end(), "(", ')', buff);
  }

  native_persistent_string native_vector_sequence::to_string() const
  {
    util::string_builder buff;
    runtime::to_string(data.begin(), data.end(), "(", ')', buff);
    return buff.release();
  }

  native_persistent_string native_vector_sequence::to_code_string() const
  {
    util::string_builder buff;
    runtime::to_code_string(data.begin(), data.end(), "(", ')', buff);
    return buff.release();
  }

  native_hash native_vector_sequence::to_hash()
  {
    return hash::ordered(data.begin(), data.end());
  }

  /* behavior::seqable */
  native_vector_sequence_ptr native_vector_sequence::seq()
  {
    return data.empty() ? nullptr : this;
  }

  native_vector_sequence_ptr native_vector_sequence::fresh_seq() const
  {
    return data.empty() ? nullptr : make_box<native_vector_sequence>(data, index);
  }

  /* behavior::countable */
  size_t native_vector_sequence::count() const
  {
    return data.size() - index;
  }

  /* behavior::sequence */
  object_ptr native_vector_sequence::first() const
  {
    assert(index < data.size());
    return data[index];
  }

  native_vector_sequence_ptr native_vector_sequence::next() const
  {
    auto n(index);
    ++n;

    if(n == data.size())
    {
      return nullptr;
    }

    return make_box<native_vector_sequence>(data, n);
  }

  native_vector_sequence_ptr native_vector_sequence::next_in_place()
  {
    ++index;

    if(index == data.size())
    {
      return nullptr;
    }

    return this;
  }

  cons_ptr native_vector_sequence::conj(object_ptr const head)
  {
    return make_box<cons>(head, data.empty() ? nullptr : this);
  }

  native_vector_sequence_ptr native_vector_sequence::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }

}
