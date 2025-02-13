#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/obj/chunked_cons.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/behavior/chunkable.hpp>

namespace jank::runtime::obj
{
  chunked_cons::chunked_cons(object_ptr const head, object_ptr const tail)
    : head{ head }
    , tail{ tail == nil::nil_const() ? nullptr : tail }
  {
    assert(head);
  }

  chunked_cons::chunked_cons(object_ptr const meta, object_ptr const head, object_ptr const tail)
    : head{ head }
    , tail{ tail == nil::nil_const() ? nullptr : tail }
    , meta{ meta }
  {
    assert(head);
    assert(meta);
  }

  chunked_cons_ptr chunked_cons::seq() const
  {
    return const_cast<chunked_cons *>(this);
  }

  chunked_cons_ptr chunked_cons::fresh_seq() const
  {
    return make_box<chunked_cons>(head, tail);
  }

  object_ptr chunked_cons::first() const
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

  object_ptr chunked_cons::next() const
  {
    return visit_object(
      [&](auto const typed_head) -> object_ptr {
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
          throw std::runtime_error{ fmt::format("invalid chunked cons head: {}",
                                                typed_head->to_string()) };
        }
      },
      head);
  }

  static chunked_cons_ptr next_in_place_non_chunked(chunked_cons_ptr const o)
  {
    if(!o->tail)
    {
      return nullptr;
    }

    return visit_object(
      [&](auto const typed_tail) -> chunked_cons_ptr {
        using T = typename decltype(typed_tail)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          o->head = typed_tail->first();
          o->tail = typed_tail->next();
          if(o->tail == nil::nil_const())
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

  chunked_cons_ptr chunked_cons::next_in_place()
  {
    return visit_object(
      [&](auto const typed_head) -> chunked_cons_ptr {
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

  object_ptr chunked_cons::chunked_first() const
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
          auto const buffer(make_box<chunk_buffer>(static_cast<size_t>(1)));
          buffer->append(typed_head);
          return buffer->chunk();
        }
      },
      head);
  }

  object_ptr chunked_cons::chunked_next() const
  {
    return tail;
  }

  native_bool chunked_cons::equal(object const &o) const
  {
    return visit_seqable(
      [this](auto const typed_o) {
        auto seq(typed_o->fresh_seq());
        for(auto it(fresh_seq()); it != nullptr;
            it = runtime::next_in_place(it), seq = runtime::next_in_place(seq))
        {
          if(seq == nullptr || !runtime::equal(it->first(), seq->first()))
          {
            return false;
          }
        }
        return seq == nullptr;
      },
      []() { return false; },
      &o);
  }

  void chunked_cons::to_string(util::string_builder &buff) const
  {
    runtime::to_string(seq(), buff);
  }

  native_persistent_string chunked_cons::to_string() const
  {
    return runtime::to_string(seq());
  }

  native_persistent_string chunked_cons::to_code_string() const
  {
    return runtime::to_code_string(seq());
  }

  native_hash chunked_cons::to_hash() const
  {
    return hash::ordered(&base);
  }

  cons_ptr chunked_cons::conj(object_ptr const head) const
  {
    return make_box<cons>(head, this);
  }

  chunked_cons_ptr chunked_cons::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(fresh_seq());
    ret->meta = meta;
    return ret;
  }
}
