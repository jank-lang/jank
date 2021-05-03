#include <iostream>
#include <sstream>

#include <prelude/seq.hpp>
#include <prelude/util.hpp>
#include <prelude/fn.hpp>

namespace jank
{
  /***** string *****/
  detail::boolean_type string::equal(object const &o) const
  {
    auto const *s(o.as_string());
    if(!s)
    { return false; }

    return data == s->data;
  }
  detail::string_type string::to_string() const
  { return data; }
  detail::integer_type string::to_hash() const
  { return data.to_hash(); }
  string const* string::as_string() const
  { return this; }

  template <typename It>
  struct basic_iterator_wrapper : sequence, pool_item_base<basic_iterator_wrapper<It>>
  {
    basic_iterator_wrapper() = default;
    basic_iterator_wrapper(It const &b, It const &e)
      : begin{ b }, end { e }
    { }

    object_ptr first() const override
    { return *begin; }
    sequence_pointer next() const override
    {
      auto n(begin);
      ++n;

      if(n == end)
      { return nullptr; }

      return make_box<basic_iterator_wrapper<It>>(n, end);
    }

    It begin, end;
  };

  /***** vector *****/
  detail::boolean_type vector::equal(object const &o) const
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
  detail::string_type vector::to_string() const
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
  detail::integer_type vector::to_hash() const
  {
    size_t seed{ data.size() };
    for(auto const &e : data)
    { seed = jank::detail::hash_combine(seed, *e); }
    return seed;
  }
  vector const* vector::as_vector() const
  { return this; }
  seqable const* vector::as_seqable() const
  { return this; }
  sequence_pointer vector::seq() const
  {
    if(data.size() == 0)
    { return nullptr; }
    return make_box<basic_iterator_wrapper<detail::vector_type::iterator>>(data.begin(), data.end());
  }

  template <typename It>
  struct map_iterator_wrapper : sequence, pool_item_base<map_iterator_wrapper<It>>
  {
    map_iterator_wrapper() = default;
    map_iterator_wrapper(It const &b, It const &e)
      : begin{ b }
      , end{ e }
    { }

    object_ptr first() const override
    { return make_box<vector>(detail::vector_type{ begin->first, begin->second }); }
    sequence_pointer next() const override
    {
      auto n(begin);
      ++n;

      if(n == end)
      { return nullptr; }

      return make_box<map_iterator_wrapper<It>>(n, end);
    }

    It begin, end;
  };

  /***** map *****/
  detail::boolean_type map::equal(object const &o) const
  {
    auto const *m(o.as_map());
    if(!m)
    { return false; }

    return m->data == data;
  }
  detail::string_type map::to_string() const
  {
    auto const end(data.end());

    std::stringstream ss;
    ss << "{";
    for(auto i(data.begin()); i != end; ++i)
    {
      ss << *i->first << " " << *i->second;
      auto n(i);
      if(++n != end)
      { ss << " "; }
    }
    ss << "}";
    return ss.str();
  }
  detail::integer_type map::to_hash() const
  {
    size_t seed{ data.size() };
    for(auto const &e : data)
    {
      seed = jank::detail::hash_combine(seed, *e.first);
      seed = jank::detail::hash_combine(seed, *e.second);
    }
    return seed;
  }
  map const* map::as_map() const
  { return this; }
  seqable const* map::as_seqable() const
  { return this; }
  sequence_pointer map::seq() const
  {
    if(data.size() == 0)
    { return nullptr; }
    return make_box<map_iterator_wrapper<detail::map_type::iterator>>(data.begin(), data.end());
  }

  ///***** set *****/
  //detail::boolean_type set::equal(object const &) const
  //{
  //}
  //detail::string_type set::to_string() const
  //{
  //}
  //detail::integer_type set::to_hash() const
  //{
  //}
  //set const* set::as_set() const
  //{ return this; }
  //seqable const* set::as_seqable() const
  //{ return this; }
  //iterator_ptr set::begin() const
  //{ return make_box<basic_iterator_wrapper<detail::set_type::iterator>>(data.begin()); }
  //iterator_ptr set::end() const
  //{ return make_box<basic_iterator_wrapper<detail::set_type::iterator>>(data.end()); }

  /* TODO: Laziness. */
  object_ptr mapv(object_ptr const &f, object_ptr const &seq)
  {
    auto const * const sable(seq->as_seqable());
    if(!sable)
    {
      /* TODO: Throw error. */
      std::cout << "(mapv) not a seq: " << *seq << std::endl;
      return JANK_NIL;
    }

    auto const * const func(f->as_callable());
    if(!func)
    {
      /* TODO: Throw error. */
      std::cout << "(mapv) not callable: " << *f << std::endl;
      return JANK_NIL;
    }

    detail::vector_transient_type ret;

    for(auto s(sable->seq()); s != nullptr; s = s->next())
    { ret.push_back(func->call(s->first())); }

    return make_box<vector>(ret.persistent());
  }

  object_ptr reduce(object_ptr const &f, object_ptr const &initial, object_ptr const &seq)
  {
    auto const * const sable(seq->as_seqable());
    if(!sable)
    {
      /* TODO: Throw error. */
      std::cout << "(mapv) not a seq: " << *seq << std::endl;
      return JANK_NIL;
    }

    auto const * const func(f->as_callable());
    if(!func)
    {
      /* TODO: Throw error. */
      std::cout << "(mapv) not callable: " << *f << std::endl;
      return JANK_NIL;
    }

    object_ptr acc{ initial };

    for(auto s(sable->seq()); s != nullptr; s = s->next())
    { acc = func->call(acc, s->first()); }

    return acc;
  }

  /* TODO: Laziness */
  object_ptr partition_gen_minus_all(object_ptr const &n, object_ptr const &seq)
  {
    auto const * const i(n->as_integer());
    if(!i)
    {
      /* TODO: Throw error */
      std::cout << "(partition-all) size must be an integer: " << *n << std::endl;
      return JANK_NIL;
    }
    auto const partition_size(i->data);

    auto const * const sable(seq->as_seqable());
    if(!sable)
    {
      /* TODO: Throw error. */
      std::cout << "(partition-all) not a seq: " << *seq << std::endl;
      return JANK_NIL;
    }

    detail::vector_transient_type ret;

    for(auto s(sable->seq()); s != nullptr;)
    {
      detail::vector_transient_type partition;

      for(size_t k{}; k < partition_size && s != nullptr; ++k, s = s->next())
      { partition.push_back(s->first()); }

      ret.push_back(make_box<vector>(partition.persistent()));
    }

    return make_box<vector>(ret.persistent());
  }

  /* TODO: Laziness */
  object_ptr range(object_ptr const &start, object_ptr const &end)
  {
    auto const * const s(start->as_integer());
    auto const * const e(end->as_integer());
    if(!s || !e)
    {
      /* TODO: Throw error */
      std::cout << "(range) start and end must be integers: " << *start << " and " << *end << std::endl;
      return JANK_NIL;
    }

    auto const start_int(s->data);
    auto const end_int(e->data);

    if(end_int < start_int)
    {
      /* TODO: throw error */
      std::cout << "(range start must be < end" << std::endl;
      return JANK_NIL;
    }

    detail::vector_transient_type ret;
    for(auto i(start_int); i < end_int; ++i)
    { ret.push_back(make_box<integer>(i)); }
    return make_box<vector>(ret.persistent());
  }

  object_ptr reverse(object_ptr const &seq)
  {
    auto const * const sable(seq->as_seqable());
    if(!sable)
    {
      /* TODO: Throw error. */
      std::cout << "(reverse) not a seq: " << *seq << std::endl;
      return JANK_NIL;
    }

    /* TODO: Optimize this by supporting a better interface. */
    detail::vector_transient_type in_order, reverse_order;

    for(auto s(sable->seq()); s != nullptr; s = s->next())
    { in_order.push_back(s->first()); }
    for(auto it(in_order.rbegin()); it != in_order.rend(); ++it)
    { reverse_order.push_back(std::move(*it)); }

    return make_box<vector>(reverse_order.persistent());
  }

  /* TODO: Associative interface. */
  object_ptr get(object_ptr const &o, object_ptr const &key)
  {
    auto const * const m(o->as_map());
    if(m)
    {
      if(auto * const found = m->data.find(key))
      { return *found; }
      else
      { return JANK_NIL; }
    }

    auto const * const v(o->as_vector());
    if(v)
    {
      auto const * const n(key->as_integer());
      if(!n)
      {
        /* TODO: Throw error. */
        std::cout << "(get) invalid vector index: " << *key << std::endl;
        return JANK_NIL;
      }

      auto const size(v->data.size());
      if(n->data < 0 || n->data >= size)
      { return JANK_NIL; }

      return v->data[n->data];
    }

    std::cout << "(get) not associative: " << *o << " with key: " << *key << std::endl;
    return JANK_NIL;
  }

  /* TODO: Persistent collection interface. */
  object_ptr conj(object_ptr const &o, object_ptr const &val)
  {
    auto const * const m(o->as_map());
    if(m)
    {
      if(auto const * const v = val->as_vector())
      {
        if(v->data.size() != 2)
        {
          /* TODO: Throw error. */
          std::cout << "(conj) invalid map entry: " << *val << std::endl;
          return JANK_NIL;
        }
        return make_box<map>(m->data.set(v->data[0], v->data[1]));
      }
      else
      { return JANK_NIL; }
    }

    auto const * const v(o->as_vector());
    if(v)
    { return make_box<vector>(v->data.push_back(val)); }

    std::cout << "(conj) unsupported for: " << *o << std::endl;
    return JANK_NIL;
  }

  object_ptr assoc(object_ptr const &o, object_ptr const &key, object_ptr const &val)
  {
    auto const * const m(o->as_map());
    if(m)
    { return make_box<map>(m->data.set(key, val)); }

    auto const * const v(o->as_vector());
    if(v)
    {
      auto const * const n(key->as_integer());
      if(!n)
      {
        /* TODO: Throw error. */
        std::cout << "(assoc) invalid vector index: " << *key << std::endl;
        return JANK_NIL;
      }

      auto const size(v->data.size());
      if(n->data < 0 || n->data > size)
      {
        /* TODO: Throw error. */
        std::cout << "(assoc) index out of bounds: " << *key << std::endl;
        return JANK_NIL;
      }
      else if(n->data == size)
      { return make_box<vector>(v->data.push_back(val)); }

      return make_box<vector>(v->data.set(n->data, val));
    }

    std::cout << "(get) not associative: " << *o << std::endl;
    return JANK_NIL;
  }
}
