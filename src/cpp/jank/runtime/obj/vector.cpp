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
    static constexpr bool pointer_free{ false };

    vector_sequence() = default;
    vector_sequence(vector_ptr v)
      : vec{ v }
    { }
    vector_sequence(vector_ptr v, size_t i)
      : vec{ v }, index{ i }
    { }

    void to_string(fmt::memory_buffer &buff) const final
    {
      return behavior::detail::to_string
      (
        vec->data.begin() + static_cast<decltype(vector::data)::difference_type>(index),
        vec->data.end(),
        '[', ']',
        buff
      );
    }
    native_string to_string() const final
    {
      fmt::memory_buffer buff;
      behavior::detail::to_string
      (
        vec->data.begin() + static_cast<decltype(vector::data)::difference_type>(index),
        vec->data.end(),
        '[', ']',
        buff
      );
      return { buff.data(), buff.size() };
    }
    native_integer to_hash() const final
    /* TODO: Hash from contents. */
    { return reinterpret_cast<native_integer>(this); }

    sequence_ptr seq() const final
    { return static_cast<sequence_ptr>(const_cast<vector_sequence*>(this)); }
    behavior::sequence_ptr fresh_seq() const final
    { return jank::make_box<vector_sequence>(vec, index); }

    behavior::countable const* as_countable() const final
    { return this; }
    size_t count() const final
    { return vec->data.size(); }

    object_ptr first() const final
    { return vec->data[index]; }
    sequence_ptr next() const final
    {
      auto n(index);
      ++n;

      if(n == vec->data.size())
      { return nullptr; }

      return jank::make_box<vector_sequence>(vec, n);
    }
    sequence_ptr next_in_place() final
    {
      ++index;

      if(index == vec->data.size())
      { return nullptr; }

      return this;
    }
    object_ptr next_in_place_first() final
    {
      ++index;

      if(index == vec->data.size())
      { return nullptr; }

      return vec->data[index];
    }

    vector_ptr vec{};
    size_t index{};
  };

  vector::vector(runtime::detail::peristent_vector &&d)
    : data{ std::move(d) }
  { }
  vector::vector(runtime::detail::peristent_vector const &d)
    : data{ d }
  { }

  vector_ptr vector::create(runtime::detail::peristent_vector const &o)
  { return jank::make_box<vector>(o); }

  vector_ptr vector::create(behavior::sequence_ptr const &s)
  {
    if(s == nullptr)
    { return jank::make_box<vector>(); }

    runtime::detail::transient_vector v;
    v.push_back(s->first());
    for(auto i(s->next()); i != nullptr; i = i->next_in_place())
    { v.push_back(i->first()); }
    return jank::make_box<vector>(v.persistent());
  }

  native_bool vector::equal(object const &o) const
  {
    auto const *s(o.as_seqable());
    if(!s)
    { return false; }

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
  native_string vector::to_string() const
  {
    fmt::memory_buffer buff;
    behavior::detail::to_string(data.begin(), data.end(), '[', ']', buff);
    return native_string{ buff.data(), buff.size() };
  }
  /* TODO: Cache this. */
  native_integer vector::to_hash() const
  {
    auto seed(static_cast<native_integer>(data.size()));
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
    return jank::make_box<vector_sequence>(const_cast<vector*>(this));
  }
  behavior::sequence_ptr vector::fresh_seq() const
  {
    if(data.empty())
    { return nullptr; }
    return jank::make_box<vector_sequence>(const_cast<vector*>(this));
  }
  size_t vector::count() const
  { return data.size(); }

  behavior::consable const* vector::as_consable() const
  { return this; }
  native_box<behavior::consable> vector::cons(object_ptr head) const
  {
    auto vec(data.push_back(head));
    auto ret(create(std::move(vec)));
    return ret;
  }

  object_ptr vector::with_meta(object_ptr const m) const
  {
    auto const meta(validate_meta(m));
    auto ret(jank::make_box<vector>(data));
    ret->meta = meta;
    return ret;
  }

  behavior::metadatable const* vector::as_metadatable() const
  { return this; }

  behavior::associatively_readable const* vector::as_associatively_readable() const
  { return this; }
  object_ptr vector::get(object_ptr const key) const
  {
    if(auto const i = key->as_integer())
    {
      if(data.size() <= static_cast<size_t>(i->data))
      { return JANK_NIL; }
      return data[i->data];
    }
    else
    {
      throw std::runtime_error
      { fmt::format("get on a vector must be an integer; found {}", key->to_string()) };
    }
  }
  object_ptr vector::get(object_ptr const key, object_ptr const fallback) const
  {
    if(auto const i = key->as_integer())
    {
      if(data.size() <= static_cast<size_t>(i->data))
      { return fallback; }
      return data[i->data];
    }
    else
    {
      throw std::runtime_error
      { fmt::format("get on a vector must be an integer; found {}", key->to_string()) };
    }
  }
}
