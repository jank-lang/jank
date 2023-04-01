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
    size_t sequence_length(behavior::sequence_ptr const s)
    { return sequence_length(s, std::numeric_limits<size_t>::max()); }
    size_t sequence_length(behavior::sequence_ptr const s, size_t const max)
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

    native_string to_string(behavior::sequence_ptr const s)
    {
      fmt::memory_buffer buff;
      to_string(s, buff);
      return native_string{ buff.data(), buff.size() };
    }
    void to_string(behavior::sequence_ptr const s, fmt::memory_buffer &buff)
    {
      auto inserter(std::back_inserter(buff));
      if(!s)
      { fmt::format_to(inserter, "()"); }

      fmt::format_to(inserter, "(");
      bool needs_space{};
      for(auto i(s); i != nullptr; i = i->next())
      {
        if(needs_space)
        { fmt::format_to(inserter, " "); }
        i->first()->to_string(buff);
        needs_space = true;
      }
      fmt::format_to(inserter, ")");
    }
  }

  object_ptr seq(object_ptr const s)
  {
    if(s->as_nil())
    { return s; }

    auto const * const sable(s->as_seqable());
    if(!sable)
    { throw std::runtime_error{ fmt::format("not seqable: {}", s->to_string()) }; }

    auto const &ret(sable->seq());
    if(!ret)
    { return JANK_NIL; }

    return ret;
  }

  object_ptr first(object_ptr const s)
  {
    if(s == JANK_NIL)
    { return s; }

    auto const * const seqable(s->as_seqable());
    if(!seqable)
    { throw std::runtime_error{ fmt::format("not seqable: {}", s->to_string()) }; }

    auto const seq(seqable->seq());
    if(!seq)
    { return JANK_NIL; }

    return seq->first();
  }

  object_ptr first(behavior::seqable_ptr const s)
  {
    auto const seq(s->seq());
    if(!seq)
    { return JANK_NIL; }

    return seq->first();
  }

  object_ptr next(object_ptr const s)
  {
    if(s == JANK_NIL)
    { return s; }

    auto const * const seqable(s->as_seqable());
    if(!seqable)
    { throw std::runtime_error{ fmt::format("not seqable: {}", s->to_string()) }; }

    auto const seq(seqable->seq());
    if(!seq)
    { return JANK_NIL; }
    else
    {
      auto const ret(seq->next());
      if(!ret)
      { return JANK_NIL; }
      return ret;
    }
  }

  object_ptr next(behavior::seqable_ptr const s)
  {
    auto const seq(s->seq());
    if(!seq)
    { return JANK_NIL; }
    else
    {
      auto const ret(seq->next());
      if(!ret)
      { return JANK_NIL; }
      return ret;
    }
  }

  object_ptr conj(object_ptr const s, object_ptr const o)
  {
    if(s->as_nil())
    { return make_box<jank::runtime::obj::list>(o); }
    else if(auto const consable = s->as_consable())
    { return consable->cons(o); }
    else if(auto const seqable = s->as_seqable())
    { return seqable->seq()->cons(o); }
    else
    { throw std::runtime_error{ fmt::format("not seqable: {}", s->to_string()) }; }
  }

  object_ptr assoc(object_ptr const m, object_ptr const k, object_ptr const v)
  {
    auto const res(m->as_associatively_writable());
    if(!res)
    { throw std::runtime_error{ fmt::format("not associatively writable: {}", m->to_string()) }; }
    return res->assoc(k, v);
  }

  object_ptr get(object_ptr const o, object_ptr const key)
  {
    assert(o);
    auto const res(o->as_associatively_readable());
    if(!res)
    { throw std::runtime_error{ fmt::format("not associatively readable: {}", o->to_string()) }; }
    return res->get(key);
  }

  object_ptr get(object_ptr const o, object_ptr const key, object_ptr const fallback)
  {
    assert(o);
    auto const res(o->as_associatively_readable());
    if(!res)
    { throw std::runtime_error{ fmt::format("not associatively readable: {}", o->to_string()) }; }
    return res->get(key, fallback);
  }
}
