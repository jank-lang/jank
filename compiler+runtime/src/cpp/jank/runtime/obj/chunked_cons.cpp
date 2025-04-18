#include <jank/runtime/obj/chunked_cons.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/behavior/chunkable.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  chunked_cons::chunked_cons(object_ref const head, object_ref const tail)
    : head{ head }
    , tail{ tail }
  {
    jank_debug_assert(head.is_some());
  }

  chunked_cons::chunked_cons(object_ref const meta, object_ref const head, object_ref const tail)
    : head{ head }
    , tail{ tail }
    , meta{ meta }
  {
    jank_debug_assert(head.is_some());
    jank_debug_assert(meta.is_some());
  }

  chunked_cons_ref chunked_cons::seq() const
  {
    return const_cast<chunked_cons *>(this);
  }

  chunked_cons_ref chunked_cons::fresh_seq() const
  {
    return make_box<chunked_cons>(head, tail);
  }

  object_ref chunked_cons::first() const
  {
    return visit_object(
      [&](auto const typed_head) -> object_ref {
        using T = typename decltype(typed_head)::value_type;

        if constexpr(behavior::chunk_like<T>)
        {
          return typed_head->nth(make_box(0));
        }
        else
        {
          throw std::runtime_error{ util::format("invalid chunked cons head: {}",
                                                 typed_head->to_string()) };
        }
      },
      head);
  }

  object_ref chunked_cons::next() const
  {
    return visit_object(
      [&](auto const typed_head) -> object_ref {
        using T = typename decltype(typed_head)::value_type;

        if constexpr(behavior::chunk_like<T>)
        {
          if(1 < typed_head->count())
          {
            return make_box<chunked_cons>(typed_head->chunk_next(), tail);
          }
          return tail;
        }
        else
        {
          throw std::runtime_error{ util::format("invalid chunked cons head: {}",
                                                 typed_head->to_string()) };
        }
      },
      head);
  }

  static chunked_cons_ref next_in_place_non_chunked(chunked_cons_ref const o)
  {
    if(o->tail.is_nil())
    {
      return {};
    }

    return visit_object(
      [&](auto const typed_tail) -> chunked_cons_ref {
        using T = typename decltype(typed_tail)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          o->head = typed_tail->first();
          o->tail = typed_tail->next();
          if(o->tail == jank_nil)
          {
            o->tail = jank_nil;
          }
          return o;
        }
        else
        {
          throw std::runtime_error{ util::format("invalid sequence: {}", typed_tail->to_string()) };
        }
      },
      o->tail);
  }

  chunked_cons_ref chunked_cons::next_in_place()
  {
    return visit_object(
      [&](auto const typed_head) -> chunked_cons_ref {
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

  object_ref chunked_cons::chunked_first() const
  {
    return visit_object(
      [&](auto const typed_head) -> object_ref {
        using T = typename decltype(typed_head)::value_type;

        if constexpr(behavior::chunk_like<T>)
        {
          return typed_head;
        }
        else
        {
          auto const buffer(make_box<chunk_buffer>(static_cast<size_t>(1)));
          buffer->append(typed_head);
          return buffer->chunk();
        }
      },
      head);
  }

  object_ref chunked_cons::chunked_next() const
  {
    return tail;
  }

  bool chunked_cons::equal(object const &o) const
  {
    return runtime::sequence_equal(this, &o);
  }

  void chunked_cons::to_string(util::string_builder &buff) const
  {
    runtime::to_string(seq(), buff);
  }

  jtl::immutable_string chunked_cons::to_string() const
  {
    return runtime::to_string(seq());
  }

  jtl::immutable_string chunked_cons::to_code_string() const
  {
    return runtime::to_code_string(seq());
  }

  native_hash chunked_cons::to_hash() const
  {
    return hash::ordered(&base);
  }

  cons_ref chunked_cons::conj(object_ref const head) const
  {
    return make_box<cons>(head, this);
  }

  chunked_cons_ref chunked_cons::with_meta(object_ref const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }
}
