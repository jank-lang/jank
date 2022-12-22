#include <iostream>
#include <sstream>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/list.hpp>

namespace jank::runtime::obj
{
  list::list(runtime::detail::list_type &&d)
    : data{ std::move(d) }
  { }
  list::list(runtime::detail::list_type const &d)
    : data{ d }
  { }

  list_ptr list::create(runtime::detail::list_type const &l)
  { return make_box<list>(l); }
  list_ptr list::create(behavior::sequence_ptr const &s)
  {
    if(s == nullptr)
    { return make_box<list>(); }

    std::vector<object_ptr> v;
    v.emplace_back(s->first());
    for(auto i(s->next()); i != nullptr; i = i->next_in_place())
    { v.emplace_back(i->first()); }
    return make_box<list>(runtime::detail::list_type{ v.rbegin(), v.rend() });
  }

  runtime::detail::boolean_type list::equal(object const &o) const
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
  runtime::detail::string_type list::to_string() const
  {
    auto const end(data.end());
    std::stringstream ss;
    ss << "(";
    for(auto i(data.begin()); i != end; ++i)
    {
      ss << **i;
      auto n(i);
      if(++n != end)
      { ss << " "; }
    }
    ss << ")";
    return ss.str();
  }
  /* TODO: Cache this. */
  runtime::detail::integer_type list::to_hash() const
  {
    auto seed(static_cast<runtime::detail::integer_type>(data.size()));
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
    return make_box
    <
      behavior::basic_iterator_wrapper<runtime::detail::list_type::iterator>
    >(ptr_from_this(), data.begin(), data.end(), data.size());
  }
  size_t list::count() const
  { return data.size(); }

  object_ptr list::with_meta(object_ptr const &m) const
  {
    validate_meta(m);
    auto ret(make_box<list>(data));
    ret->meta = m;
    return ret;
  }

  behavior::metadatable const* list::as_metadatable() const
  { return this; }
}
