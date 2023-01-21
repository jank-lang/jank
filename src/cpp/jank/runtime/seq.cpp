#include <iostream>
#include <sstream>

#include <fmt/core.h>

#include <jank/runtime/seq.hpp>
#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/vector.hpp>
#include <jank/runtime/obj/map.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime
{
  namespace detail
  {
    size_t sequence_length(behavior::sequence_ptr const &s)
    { return sequence_length(s, std::numeric_limits<size_t>::max()); }
    size_t sequence_length(behavior::sequence_ptr const &s, size_t const max)
    {
      if(s == nullptr)
      { return 0; }
      else if(auto const * const c = s->as_countable())
      { return c->count(); }

      size_t length{ 1 };
      for(auto i(s->next()); i != nullptr && length < max; i = i->next_in_place())
      { ++length; }
      return length;
    }
  }

  object_ptr seq(object_ptr s)
  {
    auto const * const sable(s->as_seqable());
    if(!sable)
    { throw std::runtime_error{ fmt::format("not seqable: {}", s->to_string()) }; }
    auto const &ret(sable->seq());
    if(!ret)
    { return JANK_NIL; }
    return ret;
  }

  /* TODO: Laziness. */
  object_ptr mapv(object_ptr f, object_ptr seq)
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

    detail::transient_vector ret;

    for(auto s(sable->seq()); s != nullptr; s = s->next_in_place())
    { ret.push_back(func->call(s->first())); }

    return jank::make_box<obj::vector>(ret.persistent());
  }

  object_ptr reduce(object_ptr f, object_ptr initial, object_ptr seq)
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

    for(auto s(sable->seq()); s != nullptr; s = s->next_in_place())
    { acc = func->call(acc, s->first()); }

    return acc;
  }

  /* TODO: Laziness */
  object_ptr partition_gen_minus_all(object_ptr n, object_ptr seq)
  {
    auto const * const i(n->as_integer());
    if(!i)
    {
      /* TODO: Throw error */
      std::cout << "(partition-all) size must be an integer: " << *n << std::endl;
      return JANK_NIL;
    }
    size_t const partition_size(i->data);

    auto const * const sable(seq->as_seqable());
    if(!sable)
    {
      /* TODO: Throw error. */
      std::cout << "(partition-all) not a seq: " << *seq << std::endl;
      return JANK_NIL;
    }

    detail::transient_vector ret;

    for(auto s(sable->seq()); s != nullptr;)
    {
      detail::transient_vector partition;

      for(size_t k{}; k < partition_size && s != nullptr; ++k, s = s->next_in_place())
      { partition.push_back(s->first()); }

      ret.push_back(jank::make_box<obj::vector>(partition.persistent()));
    }

    return jank::make_box<obj::vector>(ret.persistent());
  }

  /* TODO: Laziness */
  object_ptr range(object_ptr start, object_ptr end)
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

    detail::transient_vector ret;
    for(auto i(start_int); i < end_int; ++i)
    { ret.push_back(jank::make_box<obj::integer>(i)); }
    return jank::make_box<obj::vector>(ret.persistent());
  }

  object_ptr reverse(object_ptr seq)
  {
    auto const * const sable(seq->as_seqable());
    if(!sable)
    {
      /* TODO: Throw error. */
      std::cout << "(reverse) not a seq: " << *seq << std::endl;
      return JANK_NIL;
    }

    /* TODO: Optimize this by supporting a better interface. */
    detail::transient_vector in_order, reverse_order;

    for(auto s(sable->seq()); s != nullptr; s = s->next_in_place())
    { in_order.push_back(s->first()); }
    for(auto it(in_order.rbegin()); it != in_order.rend(); ++it)
    { reverse_order.push_back(*it); }

    return jank::make_box<obj::vector>(reverse_order.persistent());
  }

  /* TODO: Associative interface. */
  object_ptr get(object_ptr o, object_ptr key)
  {
    auto const * const m(o->as_map());
    if(m)
    {
      if(auto const * found = m->data.find(key))
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
      if(n->data < 0 || static_cast<size_t>(n->data) >= size)
      { return JANK_NIL; }

      return v->data[n->data];
    }

    std::cout << "(get) not associative: " << *o << " with key: " << *key << std::endl;
    return JANK_NIL;
  }

  /* TODO: Persistent collection interface. */
  object_ptr conj(object_ptr o, object_ptr val)
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
        detail::persistent_map copy{ m->data };
        copy.insert_or_assign(v->data[0], v->data[1]);
        return jank::make_box<obj::map>(std::move(copy));
      }
      else
      { return JANK_NIL; }
    }

    auto const * const v(o->as_vector());
    if(v)
    { return jank::make_box<obj::vector>(v->data.push_back(val)); }

    std::cout << "(conj) unsupported for: " << *o << std::endl;
    return JANK_NIL;
  }

  object_ptr assoc(object_ptr o, object_ptr key, object_ptr val)
  {
    auto const * const m(o->as_map());
    if(m)
    {
      detail::persistent_map copy{ m->data };
      copy.insert_or_assign(key, val);
      return jank::make_box<obj::map>(std::move(copy));
    }

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
      if(n->data < 0 || static_cast<size_t>(n->data) > size)
      {
        /* TODO: Throw error. */
        std::cout << "(assoc) index out of bounds: " << *key << std::endl;
        return JANK_NIL;
      }
      else if(static_cast<size_t>(n->data) == size)
      { return jank::make_box<obj::vector>(v->data.push_back(val)); }

      return jank::make_box<obj::vector>(v->data.set(n->data, val));
    }

    std::cout << "(get) not associative: " << *o << std::endl;
    return JANK_NIL;
  }
}
