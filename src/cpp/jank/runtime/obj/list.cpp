#include <iostream>
#include <sstream>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/list.hpp>

namespace jank::runtime::obj
{
  list::list(runtime::detail::persistent_list &&d)
    : data{ std::move(d) }
  { }
  list::list(runtime::detail::persistent_list const &d)
    : data{ d }
  { }

  list_ptr list::create(behavior::sequence_ptr const &s)
  {
    if(s == nullptr)
    { return jank::make_box<list>(); }

    native_vector<object_ptr> v;
    v.emplace_back(s->first());
    for(auto i(s->next()); i != nullptr; i = i->next_in_place())
    { v.emplace_back(i->first()); }
    return jank::make_box<list>(runtime::detail::persistent_list{ v.rbegin(), v.rend() });
  }

  native_bool list::equal(object const &o) const
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
  void list::to_string(fmt::memory_buffer &buff) const
  { return behavior::detail::to_string(data.begin(), data.end(), '(', ')', buff); }
  native_string list::to_string() const
  {
    fmt::memory_buffer buff;
    behavior::detail::to_string(data.begin(), data.end(), '(', ')', buff);
    return native_string{ buff.data(), buff.size() };
  }
  /* TODO: Cache this. */
  native_integer list::to_hash() const
  {
    auto seed(static_cast<native_integer>(data.size()));
    for(auto const &e : data)
    { seed = runtime::detail::hash_combine(seed, *e); }
    return seed;
  }
  list const* list::as_list() const
  { return this; }
  behavior::seqable const* list::as_seqable() const
  { return this; }
  behavior::sequence_ptr list::seq() const
  {
    if(data.size() == 0)
    { return nullptr; }
    return jank::make_box
    <
      behavior::basic_iterator_wrapper<runtime::detail::persistent_list::iterator>
    >(const_cast<list*>(this), data.begin(), data.end(), data.size());
  }
  size_t list::count() const
  { return data.size(); }

  behavior::consable const* list::as_consable() const
  { return this; }
  native_box<behavior::consable> list::cons(object_ptr head) const
  {
    auto l(data.cons(head));
    auto ret(jank::make_box(std::move(l))->as_list());
    return const_cast<list*>(ret);
  }

  object_ptr list::with_meta(object_ptr m) const
  {
    auto const meta(validate_meta(m));
    auto ret(jank::make_box<list>(data));
    ret->meta = meta;
    return ret;
  }

  behavior::metadatable const* list::as_metadatable() const
  { return this; }
}
