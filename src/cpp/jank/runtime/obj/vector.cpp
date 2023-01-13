#include <iostream>
#include <sstream>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/vector.hpp>

namespace jank::runtime::obj
{
  struct vector_sequence : behavior::sequence, behavior::countable
  {
    vector_sequence() = default;
    vector_sequence(vector_ptr v)
      : vec{ v }
    { }
    vector_sequence(vector_ptr v, size_t i)
      : vec{ v }, index{ i }
    { }

    void to_string(fmt::memory_buffer &buff) const override
    { return behavior::detail::to_string(vec->data.begin() + index, vec->data.end(), '[', ']', buff); }
    runtime::detail::string_type to_string() const override
    {
      fmt::memory_buffer buff;
      behavior::detail::to_string(vec->data.begin() + index, vec->data.end(), '[', ']', buff);
      return folly::fbstring{ buff.data(), buff.size() };
    }
    runtime::detail::integer_type to_hash() const override
    { return reinterpret_cast<runtime::detail::integer_type>(this); }

    behavior::seqable const* as_seqable() const override
    { return this; }
    sequence_ptr seq() const override
    { return static_cast<sequence_ptr>(const_cast<vector_sequence*>(this)); }

    behavior::countable const* as_countable() const override
    { return this; }
    size_t count() const override
    { return vec->data.size(); }

    object_ptr first() const override
    { return vec->data[index]; }
    sequence_ptr next() const override
    {
      auto n(index);
      ++n;

      if(n == vec->data.size())
      { return nullptr; }

      return make_box<vector_sequence>(vec, n);
    }
    sequence_ptr next_in_place() override
    {
      ++index;

      if(index == vec->data.size())
      { return nullptr; }

      return this;
    }
    object_ptr next_in_place_first() override
    {
      ++index;

      if(index == vec->data.size())
      { return nullptr; }

      return vec->data[index];
    }

    vector_ptr vec;
    size_t index{};
  };

  vector::vector(runtime::detail::vector_type &&d)
    : data{ std::move(d) }
  { }
  vector::vector(runtime::detail::vector_type const &d)
    : data{ d }
  { }

  vector_ptr vector::create(runtime::detail::vector_type const &o)
  { return make_box<vector>(o); }

  runtime::detail::boolean_type vector::equal(object const &o) const
  {
    auto const *s(o.as_seqable());
    if(!s)
    { return false; }

    /* TODO: Optimize using better interfaces. */
    auto seq(s->seq());
    for(auto it(data.begin()); it != data.end(); ++it, seq = seq->next_in_place())
    {
      if(seq == nullptr || !(*it)->equal(*seq->first()))
      { return false; }
    }
    return true;
  }
  void vector::to_string(fmt::memory_buffer &buff) const
  { return behavior::detail::to_string(data.begin(), data.end(), '[', ']', buff); }
  runtime::detail::string_type vector::to_string() const
  {
    fmt::memory_buffer buff;
    behavior::detail::to_string(data.begin(), data.end(), '[', ']', buff);
    return std::string{ buff.data(), buff.size() };
  }
  /* TODO: Cache this. */
  runtime::detail::integer_type vector::to_hash() const
  {
    auto seed(static_cast<runtime::detail::integer_type>(data.size()));
    for(auto const &e : data)
    { seed = runtime::detail::hash_combine(seed, *e); }
    return seed;
  }
  vector const* vector::as_vector() const
  { return this; }
  behavior::seqable const* vector::as_seqable() const
  { return this; }
  behavior::sequence_ptr vector::seq() const
  {
    if(data.empty())
    { return nullptr; }
    return make_box<vector_sequence>(const_cast<vector*>(this));
  }
  size_t vector::count() const
  { return data.size(); }

  object_ptr vector::with_meta(object_ptr m) const
  {
    validate_meta(m);
    auto ret(make_box<vector>(data));
    ret->meta = m;
    return ret;
  }

  behavior::metadatable const* vector::as_metadatable() const
  { return this; }
}
