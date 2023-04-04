#include <iostream>
#include <sstream>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/set.hpp>

namespace jank::runtime::obj
{
  set::set(runtime::detail::persistent_set &&d)
    : data{ std::move(d) }
  { }
  set::set(runtime::detail::persistent_set const &d)
    : data{ d }
  { }

  native_bool set::equal(object const &o) const
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
  void set::to_string(fmt::memory_buffer &buff) const
  { return behavior::detail::to_string(data.begin(), data.end(), '(', ')', buff); }
  native_string set::to_string() const
  {
    fmt::memory_buffer buff;
    behavior::detail::to_string(data.begin(), data.end(), '(', ')', buff);
    return native_string{ buff.data(), buff.size() };
  }
  /* TODO: Cache this. */
  native_integer set::to_hash() const
  {
    auto seed(static_cast<native_integer>(data.size()));
    for(auto const &e : data)
    { seed = runtime::detail::hash_combine(seed, *e); }
    return seed;
  }
  set const* set::as_set() const
  { return this; }
  behavior::seqable const* set::as_seqable() const
  { return this; }
  behavior::sequence_ptr set::seq() const
  {
    if(data.empty())
    { return nullptr; }
    return jank::make_box
    <
      behavior::basic_iterator_wrapper<runtime::detail::persistent_set::iterator>
    >(const_cast<set*>(this), data.begin(), data.end(), data.size());
  }
  behavior::sequence_ptr set::fresh_seq() const
  {
    if(data.empty())
    { return nullptr; }
    return jank::make_box
    <
      behavior::basic_iterator_wrapper<runtime::detail::persistent_set::iterator>
    >(const_cast<set*>(this), data.begin(), data.end(), data.size());
  }

  size_t set::count() const
  { return data.size(); }

  object_ptr set::with_meta(object_ptr const m) const
  {
    auto const meta(validate_meta(m));
    auto ret(jank::make_box<set>(data));
    ret->meta = meta;
    return ret;
  }

  behavior::metadatable const* set::as_metadatable() const
  { return this; }
}
