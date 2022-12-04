#include <iostream>
#include <sstream>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/vector.hpp>

namespace jank::runtime::obj
{
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
    for(auto it(data.begin()); it != data.end(); ++it, seq = seq->next())
    {
      if(seq == nullptr || !(*it)->equal(*seq->first()))
      { return false; }
    }
    return true;
  }
  runtime::detail::string_type vector::to_string() const
  {
    auto const end(data.end());
    std::stringstream ss;
    ss << "[";
    for(auto i(data.begin()); i != end; ++i)
    {
      ss << **i;
      auto n(i);
      if(++n != end)
      { ss << " "; }
    }
    ss << "]";
    return ss.str();
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
    return make_box<behavior::basic_iterator_wrapper<runtime::detail::vector_type::iterator>>(data.begin(), data.end());
  }
  size_t vector::count() const
  { return data.size(); }

  object_ptr vector::with_meta(object_ptr const &m) const
  {
    validate_meta(m);
    auto ret(make_box<vector>(data));
    ret->meta = m;
    return ret;
  }

  behavior::metadatable const* vector::as_metadatable() const
  { return this; }
}
