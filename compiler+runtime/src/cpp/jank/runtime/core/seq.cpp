#include <fmt/core.h>

#include <jank/runtime/behavior/associatively_readable.hpp>
#include <jank/runtime/behavior/associatively_writable.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/behavior/conjable.hpp>
#include <jank/runtime/behavior/countable.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>

namespace jank::runtime
{
  size_t sequence_length(object_ptr const s)
  {
    return sequence_length(s, std::numeric_limits<size_t>::max());
  }

  size_t sequence_length(object_ptr const s, size_t const max)
  {
    if(s == nullptr)
    {
      return 0;
    }

    return visit_object(
      [&](auto const typed_s) -> size_t {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return 0;
        }
        else if constexpr(behavior::countable<T>)
        {
          return typed_s->count();
        }
        else if constexpr(behavior::seqable<T>)
        {
          size_t length{ 0 };
          for(auto i(typed_s->fresh_seq()); i != nullptr && length < max; i = next_in_place(i))
          {
            ++length;
          }
          return length;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not seqable: {}", typed_s->to_string()) };
        }
      },
      s);
  }

  native_bool is_nil(object_ptr const o)
  {
    return (o == obj::nil::nil_const());
  }

  native_bool is_some(object_ptr const o)
  {
    return (o != obj::nil::nil_const());
  }

  native_bool is_empty(object_ptr const o)
  {
    return visit_object(
      [=](auto const typed_o) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return true;
        }
        else if constexpr(behavior::seqable<T>)
        {
          return typed_o->seq() == nullptr;
        }
        else if constexpr(behavior::countable<T>)
        {
          return typed_o->count() == 0;
        }
        else
        {
          throw std::runtime_error{ fmt::format("cannot check if this is empty: {}",
                                                typed_o->to_string()) };
        }
      },
      o);
  }

  native_bool is_seq(object_ptr const o)
  {
    return visit_object(
      [=](auto const typed_o) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        return behavior::sequenceable<T>;
      },
      o);
  }

  native_bool is_sequential(object_ptr const o)
  {
    return visit_object(
      [=](auto const typed_o) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        return behavior::sequential<T>;
      },
      o);
  }

  native_bool is_collection(object_ptr const o)
  {
    return visit_object(
      [=](auto const typed_o) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        return behavior::collection_like<T>;
      },
      o);
  }

  native_bool is_map(object_ptr const o)
  {
    return (o->type == object_type::persistent_hash_map
            || o->type == object_type::persistent_array_map);
  }

  native_bool is_transientable(object_ptr const o)
  {
    return visit_object(
      [=](auto const typed_o) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        return behavior::transientable<T>;
      },
      o);
  }

  object_ptr transient(object_ptr const o)
  {
    return visit_object(
      [=](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::transientable<T>)
        {
          return typed_o->to_transient();
        }
        else
        {
          throw std::runtime_error{ fmt::format("not transientable: {}", typed_o->to_string()) };
        }
      },
      o);
  }

  object_ptr persistent(object_ptr const o)
  {
    return visit_object(
      [=](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::persistentable<T>)
        {
          return typed_o->to_persistent();
        }
        else
        {
          throw std::runtime_error{ fmt::format("not persistentable: {}", typed_o->to_string()) };
        }
      },
      o);
  }

  object_ptr conj_in_place(object_ptr const coll, object_ptr const o)
  {
    return visit_object(
      [](auto const typed_coll, auto const o) -> object_ptr {
        using T = typename decltype(typed_coll)::value_type;

        if constexpr(behavior::conjable_in_place<T>)
        {
          return typed_coll->conj_in_place(o);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not conjable_in_place: {}",
                                                typed_coll->to_string()) };
        }
      },
      coll,
      o);
  }

  object_ptr assoc_in_place(object_ptr const coll, object_ptr const k, object_ptr const v)
  {
    return visit_object(
      [](auto const typed_coll, auto const k, auto const v) -> object_ptr {
        using T = typename decltype(typed_coll)::value_type;

        if constexpr(behavior::associatively_writable_in_place<T>)
        {
          return typed_coll->assoc_in_place(k, v);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not associatively_writable_in_place: {}",
                                                typed_coll->to_string()) };
        }
      },
      coll,
      k,
      v);
  }

  object_ptr dissoc_in_place(object_ptr const coll, object_ptr const k)
  {
    return visit_object(
      [](auto const typed_coll, auto const k) -> object_ptr {
        using T = typename decltype(typed_coll)::value_type;

        if constexpr(behavior::associatively_writable_in_place<T>)
        {
          return typed_coll->dissoc_in_place(k);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not associatively_writable_in_place: {}",
                                                typed_coll->to_string()) };
        }
      },
      coll,
      k);
  }

  object_ptr seq(object_ptr const s)
  {
    return visit_object(
      [](auto const typed_s) -> object_ptr {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return typed_s;
        }
        else if constexpr(behavior::seqable<T>)
        {
          auto const ret(typed_s->seq());
          if(!ret)
          {
            return obj::nil::nil_const();
          }

          return ret;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not seqable: {}", typed_s->to_string()) };
        }
      },
      s);
  }

  object_ptr fresh_seq(object_ptr const s)
  {
    return visit_object(
      [](auto const typed_s) -> object_ptr {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return typed_s;
        }
        else if constexpr(behavior::seqable<T>)
        {
          auto const ret(typed_s->fresh_seq());
          if(!ret)
          {
            return obj::nil::nil_const();
          }

          return ret;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not seqable: {}", typed_s->to_string()) };
        }
      },
      s);
  }

  object_ptr first(object_ptr const s)
  {
    return visit_object(
      [](auto const typed_s) -> object_ptr {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return typed_s;
        }
        else if constexpr(behavior::sequenceable<T>)
        {
          return typed_s->first();
        }
        else if constexpr(behavior::seqable<T>)
        {
          auto const ret(typed_s->seq());
          if(!ret)
          {
            return obj::nil::nil_const();
          }

          return ret->first();
        }
        else
        {
          throw std::runtime_error{ fmt::format("not seqable: {}", typed_s->to_string()) };
        }
      },
      s);
  }

  object_ptr second(object_ptr const s)
  {
    return first(next(s));
  }

  object_ptr next(object_ptr const s)
  {
    return visit_object(
      [](auto const typed_s) -> object_ptr {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return typed_s;
        }
        else if constexpr(behavior::sequenceable<T>)
        {
          return typed_s->next() ?: obj::nil::nil_const();
        }
        else if constexpr(behavior::seqable<T>)
        {
          auto const s(typed_s->seq());
          if(!s)
          {
            return obj::nil::nil_const();
          }

          return s->next() ?: obj::nil::nil_const();
        }
        else
        {
          throw std::runtime_error{ fmt::format("not seqable: {}", typed_s->to_string()) };
        }
      },
      s);
  }

  object_ptr next_in_place(object_ptr const s)
  {
    return visit_object(
      [](auto const typed_s) -> object_ptr {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return typed_s;
        }
        else if constexpr(behavior::sequenceable_in_place<T>)
        {
          return typed_s->next_in_place() ?: obj::nil::nil_const();
        }
        else if constexpr(behavior::sequenceable<T>)
        {
          return typed_s->next() ?: obj::nil::nil_const();
        }
        else if constexpr(behavior::seqable<T>)
        {
          auto const ret(typed_s->seq());
          if(!ret)
          {
            return obj::nil::nil_const();
          }

          return next_in_place(ret) ?: obj::nil::nil_const();
        }
        else
        {
          throw std::runtime_error{ fmt::format("not seqable: {}", typed_s->to_string()) };
        }
      },
      s);
  }

  object_ptr rest(object_ptr const s)
  {
    if(!s || s == obj::nil::nil_const())
    {
      return obj::persistent_list::empty();
    }
    return visit_seqable(
      [=](auto const typed_s) -> object_ptr {
        auto const seq(typed_s->seq());
        if(!seq)
        {
          return obj::persistent_list::empty();
        }
        auto const ret(seq->next());
        if(ret == nullptr)
        {
          return obj::persistent_list::empty();
        }
        return ret;
      },
      s);
  }

  object_ptr cons(object_ptr const head, object_ptr const tail)
  {
    return visit_seqable(
      [=](auto const typed_tail) -> object_ptr {
        return make_box<jank::runtime::obj::cons>(head, typed_tail->seq());
      },
      [=]() -> object_ptr {
        throw std::runtime_error{ fmt::format("not seqable: {}", runtime::to_string(tail)) };
      },
      tail);
  }

  object_ptr conj(object_ptr const s, object_ptr const o)
  {
    return visit_object(
      [&](auto const typed_s) -> object_ptr {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return make_box<obj::persistent_list>(std::in_place, o);
        }
        else if constexpr(behavior::conjable_in_place<T>)
        {
          return typed_s->conj_in_place(o);
        }
        else if constexpr(behavior::conjable<T>)
        {
          return typed_s->conj(o);
        }
        else if constexpr(behavior::seqable<T>)
        {
          return typed_s->seq()->conj(o);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not seqable: {}", typed_s->to_string()) };
        }
      },
      s);
  }

  object_ptr disj(object_ptr const s, object_ptr const o)
  {
    if(s->type == object_type::nil)
    {
      return s;
    }
    else if(s->type == object_type::persistent_hash_set)
    {
      auto const set(expect_object<obj::persistent_hash_set>(s));
      return set->disj(o);
    }
    else
    {
      throw std::runtime_error{ fmt::format("not disjoinable: {}", runtime::to_string(s)) };
    }
  }

  object_ptr assoc(object_ptr const m, object_ptr const k, object_ptr const v)
  {
    return visit_object(
      [&](auto const typed_m) -> object_ptr {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::associatively_writable_in_place<T>)
        {
          return typed_m->assoc_in_place(k, v);
        }
        else if constexpr(behavior::associatively_writable<T>)
        {
          return typed_m->assoc(k, v);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not associatively writable: {}",
                                                typed_m->to_string()) };
        }
      },
      m);
  }

  object_ptr dissoc(object_ptr const m, object_ptr const k)
  {
    return visit_object(
      [&](auto const typed_m) -> object_ptr {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::associatively_writable_in_place<T>)
        {
          return typed_m->dissoc_in_place(k);
        }
        else if constexpr(behavior::associatively_writable<T>)
        {
          return typed_m->dissoc(k);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not associatively writable: {}",
                                                typed_m->to_string()) };
        }
      },
      m);
  }

  object_ptr get(object_ptr const m, object_ptr const key)
  {
    return visit_object(
      [&](auto const typed_m) -> object_ptr {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::associatively_readable<T>)
        {
          return typed_m->get(key);
        }
        else
        {
          return obj::nil::nil_const();
        }
      },
      m);
  }

  object_ptr get(object_ptr const m, object_ptr const key, object_ptr const fallback)
  {
    return visit_object(
      [&](auto const typed_m) -> object_ptr {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::associatively_readable<T>)
        {
          return typed_m->get(key, fallback);
        }
        else
        {
          return obj::nil::nil_const();
        }
      },
      m);
  }

  object_ptr get_in(object_ptr m, object_ptr keys)
  {
    return visit_object(
      [&](auto const typed_m) -> object_ptr {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::associatively_readable<T>)
        {
          return visit_object(
            [&](auto const typed_keys) -> object_ptr {
              using T = typename decltype(typed_keys)::value_type;

              if constexpr(behavior::seqable<T>)
              {
                object_ptr ret{ typed_m };
                for(auto seq(typed_keys->fresh_seq()); seq != nullptr; seq = next_in_place(seq))
                {
                  ret = get(ret, seq->first());
                }
                return ret;
              }
              else
              {
                throw std::runtime_error{ fmt::format("not seqable: {}", typed_keys->to_string()) };
              }
            },
            keys);
        }
        else
        {
          return obj::nil::nil_const();
        }
      },
      m);
  }

  object_ptr get_in(object_ptr m, object_ptr keys, object_ptr fallback)
  {
    return visit_object(
      [&](auto const typed_m) -> object_ptr {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::associatively_readable<T>)
        {
          return visit_object(
            [&](auto const typed_keys) -> object_ptr {
              using T = typename decltype(typed_keys)::value_type;

              if constexpr(behavior::seqable<T>)
              {
                object_ptr ret{ typed_m };
                for(auto seq(typed_keys->fresh_seq()); seq != nullptr; seq = next_in_place(seq))
                {
                  ret = get(ret, seq->first());
                }

                if(ret == obj::nil::nil_const())
                {
                  return fallback;
                }
                return ret;
              }
              else
              {
                throw std::runtime_error{ fmt::format("not seqable: {}", typed_keys->to_string()) };
              }
            },
            keys);
        }
        else
        {
          return obj::nil::nil_const();
        }
      },
      m);
  }

  object_ptr find(object_ptr const s, object_ptr const key)
  {
    auto const nil(obj::nil::nil_const());
    if(s == nullptr || s == nil)
    {
      return nil;
    }

    return visit_object(
      [&](auto const typed_s) -> object_ptr {
        using S = typename decltype(typed_s)::value_type;

        if constexpr(behavior::associatively_readable<S>)
        {
          return typed_s->get_entry(key);
        }
        else
        {
          return nil;
        }
      },
      s);
  }

  native_bool contains(object_ptr const s, object_ptr const key)
  {
    if(s == nullptr || s == obj::nil::nil_const())
    {
      return false;
    }

    return visit_object(
      [&](auto const typed_s) -> native_bool {
        using S = typename decltype(typed_s)::value_type;

        if constexpr(behavior::associatively_readable<S>)
        {
          return typed_s->contains(key);
        }
        if constexpr(std::same_as<S, obj::persistent_hash_set>)
        {
          return typed_s->contains(key);
        }
        else
        {
          return false;
        }
      },
      s);
  }

  object_ptr merge(object_ptr const m, object_ptr const other)
  {
    return visit_object(
      [&](auto const typed_m) -> object_ptr {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::associatively_writable<T>)
        {
          return visit_object(
            [&](auto const typed_other) -> object_ptr {
              using O = typename decltype(typed_other)::value_type;

              if constexpr(std::same_as<O, obj::persistent_hash_map>
                           || std::same_as<O, obj::persistent_array_map>
                           || std::same_as<O, obj::transient_hash_map>
                           //|| std::same_as<O, obj::transient_array_map>
              )
              {
                object_ptr ret{ m };
                for(auto const &pair : typed_other->data)
                {
                  ret = assoc(ret, pair.first, pair.second);
                }
                return ret;
              }
              else
              {
                throw std::runtime_error{ fmt::format("not associatively readable: {}",
                                                      typed_m->to_string()) };
              }
            },
            other);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not associatively writable: {}",
                                                typed_m->to_string()) };
        }
      },
      m);
  }

  object_ptr subvec(object_ptr const o, native_integer const start, native_integer const end)
  {
    if(o->type != object_type::persistent_vector)
    {
      throw std::runtime_error{ "not a vector" };
    }

    auto const v(expect_object<obj::persistent_vector>(o));

    if(end < start || start < 0 || static_cast<size_t>(end) > v->count())
    {
      throw std::runtime_error{ "index out of bounds" };
    }
    else if(start == end)
    {
      return obj::persistent_vector::empty();
    }
    return make_box<obj::persistent_vector>(
      detail::native_persistent_vector{ v->data.begin() + start, v->data.begin() + end });
  }

  object_ptr nth(object_ptr const o, object_ptr const idx)
  {
    auto const index(to_int(idx));
    if(index < 0)
    {
      throw std::runtime_error{ fmt::format("index out of bounds: {}", index) };
    }
    else if(o == obj::nil::nil_const())
    {
      return o;
    }

    return visit_object(
      [&](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::indexable<T>)
        {
          return typed_o->nth(idx);
        }
        else if constexpr(behavior::seqable<T>)
        {
          native_integer i{};
          for(auto it(typed_o->fresh_seq()); it != nullptr; it = next_in_place(it), ++i)
          {
            if(i == index)
            {
              return it->first();
            }
          }
          throw std::runtime_error{ fmt::format("index out of bounds: {}", index) };
        }
        else
        {
          throw std::runtime_error{ fmt::format("not indexable: {}",
                                                magic_enum::enum_name(o->type)) };
        }
      },
      o);
  }

  object_ptr nth(object_ptr const o, object_ptr const idx, object_ptr const fallback)
  {
    auto const index(to_int(idx));
    if(index < 0)
    {
      return fallback;
    }
    else if(o == obj::nil::nil_const())
    {
      return o;
    }

    return visit_object(
      [&](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::indexable<T>)
        {
          return typed_o->nth(idx, fallback);
        }
        else if constexpr(behavior::seqable<T>)
        {
          native_integer i{};
          for(auto it(typed_o->fresh_seq()); it != nullptr; it = next_in_place(it), ++i)
          {
            if(i == index)
            {
              return it->first();
            }
          }
          return fallback;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not indexable: {}",
                                                magic_enum::enum_name(o->type)) };
        }
      },
      o);
  }

  object_ptr peek(object_ptr const o)
  {
    if(o == obj::nil::nil_const())
    {
      return o;
    }

    return visit_object(
      [&](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::stackable<T>)
        {
          return typed_o->peek();
        }
        else
        {
          throw std::runtime_error{ fmt::format("not stackable: {}",
                                                magic_enum::enum_name(o->type)) };
        }
      },
      o);
  }

  object_ptr pop(object_ptr const o)
  {
    if(o == obj::nil::nil_const())
    {
      return o;
    }

    return visit_object(
      [&](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::stackable<T>)
        {
          return typed_o->pop();
        }
        else
        {
          throw std::runtime_error{ fmt::format("not stackable: {}",
                                                magic_enum::enum_name(o->type)) };
        }
      },
      o);
  }

  object_ptr empty(object_ptr const o)
  {
    return visit_object(
      [=](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::collection_like<T>)
        {
          return T::empty();
        }
        else
        {
          return obj::nil::nil_const();
        }
      },
      o);
  }

  object_ptr chunk_first(object_ptr const o)
  {
    return visit_object(
      [=](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::chunkable<T>)
        {
          return typed_o->chunked_first();
        }
        {
          throw std::runtime_error{ fmt::format("not chunkable: {}", typed_o->to_string()) };
        }
      },
      o);
  }

  object_ptr chunk_next(object_ptr const o)
  {
    return visit_object(
      [=](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::chunkable<T>)
        {
          return typed_o->chunked_next() ?: obj::nil::nil_const();
        }
        {
          throw std::runtime_error{ fmt::format("not chunkable: {}", typed_o->to_string()) };
        }
      },
      o);
  }

  object_ptr chunk_rest(object_ptr const o)
  {
    return visit_object(
      [=](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::chunkable<T>)
        {
          return typed_o->chunked_next() ?: obj::persistent_list::empty();
        }
        {
          throw std::runtime_error{ fmt::format("not chunkable: {}", typed_o->to_string()) };
        }
      },
      o);
  }

  native_bool is_chunked_seq(object_ptr const o)
  {
    return visit_object(
      [=](auto const typed_o) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        return behavior::chunkable<T>;
      },
      o);
  }

  native_persistent_string str(object_ptr const o, object_ptr const args)
  {
    return visit_seqable(
      [=](auto const typed_args) -> native_persistent_string {
        fmt::memory_buffer buff;
        buff.reserve(16);
        runtime::to_string(o, buff);
        if(0 < sequence_length(typed_args))
        {
          auto const fresh(typed_args->fresh_seq());
          runtime::to_string(fresh->first(), buff);
          for(auto it(fresh->next_in_place()); it != nullptr; it = it->next_in_place())
          {
            runtime::to_string(it->first(), buff);
          }
        }
        return native_persistent_string{ buff.data(), buff.size() };
      },
      args);
  }

  obj::persistent_list_ptr list(object_ptr const s)
  {
    return visit_seqable(
      [](auto const typed_s) -> obj::persistent_list_ptr {
        return obj::persistent_list::create(typed_s);
      },
      s);
  }

  obj::persistent_vector_ptr vec(object_ptr const s)
  {
    return visit_seqable(
      [](auto const typed_s) -> obj::persistent_vector_ptr {
        return obj::persistent_vector::create(typed_s);
      },
      s);
  }

  object_ptr reduce(object_ptr const f, object_ptr const init, object_ptr const s)
  {
    return visit_seqable(
      [](auto const typed_coll, object_ptr const f, object_ptr const init) -> object_ptr {
        object_ptr res{ init };
        for(auto it(typed_coll->fresh_seq()); it != nullptr; it = it->next_in_place())
        {
          res = dynamic_call(f, res, it->first());
          if(res->type == object_type::reduced)
          {
            res = expect_object<obj::reduced>(res)->val;
            break;
          }
        }
        return res;
      },
      s,
      f,
      init);
  }
}
