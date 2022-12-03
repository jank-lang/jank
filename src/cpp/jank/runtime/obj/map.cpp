#include <iostream>
#include <sstream>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/map.hpp>
#include <jank/runtime/obj/vector.hpp>
#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  map::map(runtime::detail::map_type &&d)
    : data{ std::move(d) }
  { }
  map::map(runtime::detail::map_type const &d)
    : data{ d }
  { }

  /* TODO: Optimize this. */
  template <typename It>
  struct map_iterator_wrapper : behavior::sequence, pool_item_base<map_iterator_wrapper<It>>
  {
    map_iterator_wrapper() = default;
    map_iterator_wrapper(It const &b, It const &e)
      : begin{ b }
      , end{ e }
    { }

    object_ptr first() const override
    { return make_box<vector>(runtime::detail::vector_type{ begin->first, begin->second }); }
    behavior::sequence_ptr next() const override
    {
      auto n(begin);
      ++n;

      if(n == end)
      { return nullptr; }

      return make_box<map_iterator_wrapper<It>>(n, end);
    }

    It begin, end;
  };

  runtime::detail::boolean_type map::equal(object const &o) const
  {
    auto const *m(o.as_map());
    if(!m)
    { return false; }

    return to_hash() == m->to_hash();
  }
  runtime::detail::string_type map::to_string() const
  {
    auto const end(data.end());

    std::stringstream ss;
    ss << "{";
    for(auto i(data.begin()); i != end; ++i)
    {
      ss << *i->first << " " << *i->second;
      auto n(i);
      if(++n != end)
      { ss << ", "; }
    }
    ss << "}";
    return ss.str();
  }
  /* TODO: Cache this. */
  runtime::detail::integer_type map::to_hash() const
  {
    auto seed(static_cast<runtime::detail::integer_type>(data.size()));
    for(auto const &e : data)
    {
      seed = runtime::detail::hash_combine(seed, *e.first);
      seed = runtime::detail::hash_combine(seed, *e.second);
    }
    return seed;
  }
  map const* map::as_map() const
  { return this; }
  behavior::seqable const* map::as_seqable() const
  { return this; }
  behavior::sequence_ptr map::seq() const
  {
    if(data.size() == 0)
    { return nullptr; }
    return make_box<map_iterator_wrapper<runtime::detail::map_type::const_iterator>>(data.begin(), data.end());
  }
}
