#include <jank/runtime/obj/chunked_cons.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/behavior/chunkable.hpp>

namespace jank::runtime
{
  obj::chunked_cons::static_object(object_ptr const head, object_ptr const tail)
    : head{ head }
    , tail{ tail == obj::nil::nil_const() ? nullptr : tail }
  {
    assert(head);
  }

  obj::chunked_cons::static_object(object_ptr const meta,
                                   object_ptr const head,
                                   object_ptr const tail)
    : head{ head }
    , tail{ tail == obj::nil::nil_const() ? nullptr : tail }
    , meta{ meta }
  {
    assert(head);
    assert(meta);
  }

  obj::chunked_cons_ptr obj::chunked_cons::seq() const
  {
    return const_cast<obj::chunked_cons *>(this);
  }

  obj::chunked_cons_ptr obj::chunked_cons::fresh_seq() const
  {
    return make_box<obj::chunked_cons>(head, tail);
  }

  object_ptr obj::chunked_cons::first() const
  {
    return visit_object(
      [&](auto const typed_head) -> object_ptr {
        using T = typename decltype(typed_head)::value_type;

        if constexpr(behavior::chunk_like<T>)
        {
          return typed_head->nth(make_box(0));
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid chunked cons head: {}",
                                                typed_head->to_string()) };
        }
      },
      head);
  }

  object_ptr obj::chunked_cons::next() const
  {
    return visit_object(
      [&](auto const typed_head) -> object_ptr {
        using T = typename decltype(typed_head)::value_type;

        if constexpr(behavior::chunk_like<T>)
        {
          if(1 < typed_head->count())
          {
            return make_box<obj::chunked_cons>(typed_head->chunk_next(), tail);
          }
          return tail;
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid chunked cons head: {}",
                                                typed_head->to_string()) };
        }
      },
      head);
  }

  static obj::chunked_cons_ptr next_in_place_non_chunked(obj::chunked_cons_ptr const o)
  {
    if(!o->tail)
    {
      return nullptr;
    }

    return visit_object(
      [&](auto const typed_tail) -> obj::chunked_cons_ptr {
        using T = typename decltype(typed_tail)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          o->head = typed_tail->first();
          o->tail = typed_tail->next();
          if(o->tail == obj::nil::nil_const())
          {
            o->tail = nullptr;
          }
          return o;
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid sequence: {}", typed_tail->to_string()) };
        }
      },
      o->tail);
  }

  obj::chunked_cons_ptr obj::chunked_cons::next_in_place()
  {
    return visit_object(
      [&](auto const typed_head) -> obj::chunked_cons_ptr {
        using T = typename decltype(typed_head)::value_type;

        if constexpr(behavior::chunk_like<T>)
        {
          if(1 < typed_head->count())
          {
            head = typed_head->chunk_next();
            return this;
          }
          return next_in_place_non_chunked(this);
        }
        else
        {
          return next_in_place_non_chunked(this);
        }
      },
      head);
  }

  object_ptr obj::chunked_cons::chunked_first() const
  {
    return visit_object(
      [&](auto const typed_head) -> object_ptr {
        using T = typename decltype(typed_head)::value_type;

        if constexpr(behavior::chunk_like<T>)
        {
          return typed_head;
        }
        else
        {
          auto const buffer(make_box<obj::chunk_buffer>(static_cast<size_t>(1)));
          buffer->append(typed_head);
          return buffer->chunk();
        }
      },
      head);
  }

  object_ptr obj::chunked_cons::chunked_next() const
  {
    return tail;
  }

  native_bool obj::chunked_cons::equal(object const &o) const
  {
    return visit_seqable(
      [this](auto const typed_o) {
        auto seq(typed_o->fresh_seq());
        for(auto it(fresh_seq()); it != nullptr;
            it = runtime::next_in_place(it), seq = runtime::next_in_place(seq))
        {
          if(seq == nullptr || !runtime::equal(it, seq->first()))
          {
            return false;
          }
        }
        return true;
      },
      []() { return false; },
      &o);
  }

  void obj::chunked_cons::to_string(fmt::memory_buffer &buff) const
  {
    runtime::to_string(seq(), buff);
  }

  native_persistent_string obj::chunked_cons::to_string() const
  {
    return runtime::to_string(seq());
  }

  native_persistent_string obj::chunked_cons::to_code_string() const
  {
    return runtime::to_code_string(seq());
  }

  native_hash obj::chunked_cons::to_hash() const
  {
    return hash::ordered(&base);
  }

  obj::cons_ptr obj::chunked_cons::conj(object_ptr const head) const
  {
    return make_box<obj::cons>(head, this);
  }

  obj::chunked_cons_ptr obj::chunked_cons::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }
}
