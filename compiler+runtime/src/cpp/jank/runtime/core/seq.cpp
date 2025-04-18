#include <algorithm>
#include <random>

#include <jank/runtime/visit.hpp>
#include <jank/runtime/behavior/associatively_readable.hpp>
#include <jank/runtime/behavior/associatively_writable.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/behavior/conjable.hpp>
#include <jank/runtime/behavior/countable.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/behavior/set_like.hpp>
#include <jank/runtime/behavior/sequential.hpp>
#include <jank/runtime/behavior/collection_like.hpp>
#include <jank/runtime/behavior/transientable.hpp>
#include <jank/runtime/behavior/indexable.hpp>
#include <jank/runtime/behavior/stackable.hpp>
#include <jank/runtime/behavior/chunkable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime
{
  native_bool is_empty(object_ref const o)
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
          return typed_o->seq().is_nil();
        }
        else if constexpr(behavior::countable<T>)
        {
          return typed_o->count() == 0;
        }
        else
        {
          throw std::runtime_error{ util::format("cannot check if this is empty: {}",
                                                 typed_o->to_code_string()) };
        }
      },
      o);
  }

  native_bool is_seq(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        return behavior::sequenceable<T>;
      },
      o);
  }

  native_bool is_seqable(object_ref const o)
  {
    return visit_seqable([=](auto const) -> native_bool { return true; },
                         [=]() -> native_bool { return false; },
                         o);
  }

  native_bool is_sequential(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        return behavior::sequential<T>;
      },
      o);
  }

  native_bool is_collection(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        return behavior::collection_like<T>;
      },
      o);
  }

  native_bool is_list(object_ref const o)
  {
    /* TODO: Visit and use a behavior for this check instead.
     * It should apply to conses and others. */
    return o->type == object_type::persistent_list;
  }

  native_bool is_vector(object_ref const o)
  {
    return o->type == object_type::persistent_vector;
  }

  native_bool is_map(object_ref const o)
  {
    return (o->type == object_type::persistent_hash_map
            || o->type == object_type::persistent_array_map
            || o->type == object_type::persistent_sorted_map);
  }

  native_bool is_associative(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        return (behavior::associatively_readable<T> && behavior::associatively_writable<T>);
      },
      o);
  }

  native_bool is_set(object_ref const o)
  {
    return (o->type == object_type::persistent_hash_set
            || o->type == object_type::persistent_sorted_set);
  }

  native_bool is_counter(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        return behavior::countable<T>;
      },
      o);
  }

  native_bool is_transientable(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        return behavior::transientable<T>;
      },
      o);
  }

  native_bool is_sorted(object_ref const o)
  {
    return o->type == object_type::persistent_sorted_map
      || o->type == object_type::persistent_sorted_set;
  }

  object_ref transient(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::transientable<T>)
        {
          return typed_o->to_transient();
        }
        else
        {
          throw std::runtime_error{ util::format("not transientable: {}",
                                                 typed_o->to_code_string()) };
        }
      },
      o);
  }

  object_ref persistent(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::persistentable<T>)
        {
          return typed_o->to_persistent();
        }
        else
        {
          throw std::runtime_error{ util::format("not persistentable: {}",
                                                 typed_o->to_code_string()) };
        }
      },
      o);
  }

  object_ref conj_in_place(object_ref const coll, object_ref const o)
  {
    return visit_object(
      [](auto const typed_coll, auto const o) -> object_ref {
        using T = typename decltype(typed_coll)::value_type;

        if constexpr(behavior::conjable_in_place<T>)
        {
          return typed_coll->conj_in_place(o);
        }
        else
        {
          throw std::runtime_error{ util::format("not conjable_in_place: {}",
                                                 typed_coll->to_code_string()) };
        }
      },
      coll,
      o);
  }

  object_ref disj_in_place(object_ref const coll, object_ref const o)
  {
    /* TODO: disjoinable_in_place */
    if(coll->type == object_type::transient_hash_set)
    {
      return expect_object<obj::transient_hash_set>(coll)->disjoin_in_place(o);
    }
    else if(coll->type == object_type::transient_sorted_set)
    {
      return expect_object<obj::transient_sorted_set>(coll)->disjoin_in_place(o);
    }

    throw std::runtime_error{ util::format("not disjoinable_in_place: {}",
                                           runtime::to_code_string(coll)) };
  }

  object_ref assoc_in_place(object_ref const coll, object_ref const k, object_ref const v)
  {
    return visit_object(
      [](auto const typed_coll, auto const k, auto const v) -> object_ref {
        using T = typename decltype(typed_coll)::value_type;

        if constexpr(behavior::associatively_writable_in_place<T>)
        {
          return typed_coll->assoc_in_place(k, v);
        }
        else
        {
          throw std::runtime_error{ util::format("not associatively_writable_in_place: {}",
                                                 typed_coll->to_code_string()) };
        }
      },
      coll,
      k,
      v);
  }

  object_ref dissoc_in_place(object_ref const coll, object_ref const k)
  {
    return visit_object(
      [](auto const typed_coll, auto const k) -> object_ref {
        using T = typename decltype(typed_coll)::value_type;

        if constexpr(behavior::associatively_writable_in_place<T>)
        {
          return typed_coll->dissoc_in_place(k);
        }
        else
        {
          throw std::runtime_error{ util::format("not associatively_writable_in_place: {}",
                                                 typed_coll->to_code_string()) };
        }
      },
      coll,
      k);
  }

  object_ref pop_in_place(object_ref const coll)
  {
    auto const trans(try_object<obj::transient_vector>(coll));
    return trans->pop_in_place();
  }

  object_ref seq(object_ref const s)
  {
    return visit_object(
      [](auto const typed_s) -> object_ref {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return typed_s;
        }
        else if constexpr(behavior::seqable<T>)
        {
          auto const ret(typed_s->seq());
          if(ret.is_nil())
          {
            return ret;
          }

          return ret;
        }
        else
        {
          throw std::runtime_error{ util::format("not seqable: {}", typed_s->to_code_string()) };
        }
      },
      s);
  }

  object_ref fresh_seq(object_ref const s)
  {
    return visit_object(
      [](auto const typed_s) -> object_ref {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return typed_s;
        }
        else if constexpr(behavior::seqable<T>)
        {
          auto const ret(typed_s->fresh_seq());
          if(ret.is_nil())
          {
            return ret;
          }

          return ret;
        }
        else
        {
          throw std::runtime_error{ util::format("not seqable: {}", typed_s->to_code_string()) };
        }
      },
      s);
  }

  object_ref first(object_ref const s)
  {
    return visit_object(
      [](auto const typed_s) -> object_ref {
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
          if(ret.is_nil())
          {
            return ret;
          }

          return ret->first();
        }
        else
        {
          throw std::runtime_error{ util::format("not seqable: {}", typed_s->to_code_string()) };
        }
      },
      s);
  }

  object_ref second(object_ref const s)
  {
    return first(next(s));
  }

  object_ref next(object_ref const s)
  {
    return visit_object(
      [](auto const typed_s) -> object_ref {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return typed_s;
        }
        else if constexpr(behavior::sequenceable<T>)
        {
          return typed_s->next();
        }
        else if constexpr(behavior::seqable<T>)
        {
          auto const s(typed_s->seq());
          if(s.is_nil())
          {
            return s;
          }

          return s->next();
        }
        else
        {
          throw std::runtime_error{ util::format("not seqable: {}", typed_s->to_code_string()) };
        }
      },
      s);
  }

  object_ref next_in_place(object_ref const s)
  {
    return visit_object(
      [](auto const typed_s) -> object_ref {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return typed_s;
        }
        else if constexpr(behavior::sequenceable_in_place<T>)
        {
          return typed_s->next_in_place();
        }
        else if constexpr(behavior::sequenceable<T>)
        {
          return typed_s->next();
        }
        else if constexpr(behavior::seqable<T>)
        {
          auto const ret(typed_s->seq());
          if(ret.is_nil())
          {
            return ret;
          }

          return ret->next_in_place();
        }
        else
        {
          throw std::runtime_error{ util::format("not seqable: {}", typed_s->to_code_string()) };
        }
      },
      s);
  }

  object_ref rest(object_ref const s)
  {
    if(s.is_nil())
    {
      return obj::persistent_list::empty();
    }
    return visit_seqable(
      [=](auto const typed_s) -> object_ref {
        auto const seq(typed_s->seq());
        if(seq.is_nil())
        {
          return obj::persistent_list::empty();
        }
        auto const ret(next(seq));
        if(ret.is_nil())
        {
          return obj::persistent_list::empty();
        }
        return ret;
      },
      s);
  }

  object_ref cons(object_ref const head, object_ref const tail)
  {
    return visit_seqable(
      [=](auto const typed_tail) -> object_ref {
        using T = typename decltype(typed_tail)::value_type;

        if constexpr(jtl::is_same<T, obj::nil>)
        {
          return make_box<obj::persistent_list>(std::in_place, head);
        }
        else if constexpr(behavior::sequenceable<T>)
        {
          return make_box<jank::runtime::obj::cons>(head, typed_tail);
        }
        else
        {
          return make_box<jank::runtime::obj::cons>(head, typed_tail->seq());
        }
      },
      [=]() -> object_ref {
        throw std::runtime_error{ util::format("not seqable: {}", runtime::to_code_string(tail)) };
      },
      tail);
  }

  object_ref conj(object_ref const s, object_ref const o)
  {
    return visit_object(
      [&](auto const typed_s) -> object_ref {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return make_box<obj::persistent_list>(std::in_place, o);
        }
        else if constexpr(behavior::conjable<T>)
        {
          return typed_s->conj(o);
        }
        else
        {
          throw std::runtime_error{ util::format("not conjable: {}", typed_s->to_code_string()) };
        }
      },
      s);
  }

  object_ref disj(object_ref const s, object_ref const o)
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
      throw std::runtime_error{ util::format("not disjoinable: {}", runtime::to_code_string(s)) };
    }
  }

  object_ref assoc(object_ref const m, object_ref const k, object_ref const v)
  {
    return visit_object(
      [&](auto const typed_m) -> object_ref {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::associatively_writable<T>)
        {
          return typed_m->assoc(k, v);
        }
        else
        {
          throw std::runtime_error{ util::format("not associatively writable: {}",
                                                 typed_m->to_code_string()) };
        }
      },
      m);
  }

  object_ref dissoc(object_ref const m, object_ref const k)
  {
    return visit_object(
      [&](auto const typed_m) -> object_ref {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::associatively_writable<T>)
        {
          return typed_m->dissoc(k);
        }
        else
        {
          throw std::runtime_error{ util::format("not associatively writable: {}",
                                                 typed_m->to_code_string()) };
        }
      },
      m);
  }

  object_ref get(object_ref const m, object_ref const key)
  {
    return visit_object(
      [&](auto const typed_m) -> object_ref {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::associatively_readable<T>)
        {
          return typed_m->get(key);
        }
        else
        {
          return jank_nil;
        }
      },
      m);
  }

  object_ref get(object_ref const m, object_ref const key, object_ref const fallback)
  {
    return visit_object(
      [&](auto const typed_m) -> object_ref {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::associatively_readable<T>)
        {
          return typed_m->get(key, fallback);
        }
        else
        {
          return fallback;
        }
      },
      m);
  }

  object_ref get_in(object_ref const m, object_ref const keys)
  {
    return visit_object(
      [&](auto const typed_m) -> object_ref {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::associatively_readable<T>)
        {
          return visit_seqable(
            [&](auto const typed_keys) -> object_ref {
              object_ref ret{ typed_m };
              for(auto const e : make_sequence_range(typed_keys))
              {
                ret = get(ret, e);
              }
              return ret;
            },
            keys);
        }
        else
        {
          return jank_nil;
        }
      },
      m);
  }

  object_ref get_in(object_ref const m, object_ref const keys, object_ref const fallback)
  {
    return visit_object(
      [&](auto const typed_m) -> object_ref {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::associatively_readable<T>)
        {
          return visit_seqable(
            [&](auto const typed_keys) -> object_ref {
              object_ref ret{ typed_m };
              for(auto const e : make_sequence_range(typed_keys))
              {
                ret = get(ret, e);
              }

              if(ret == jank_nil)
              {
                return fallback;
              }
              return ret;
            },
            keys);
        }
        else
        {
          return jank_nil;
        }
      },
      m);
  }

  object_ref find(object_ref const s, object_ref const key)
  {
    if(s.is_nil())
    {
      return s;
    }

    return visit_object(
      [](auto const typed_s, object_ref const key) -> object_ref {
        using S = typename decltype(typed_s)::value_type;

        if constexpr(behavior::associatively_readable<S>)
        {
          return typed_s->get_entry(key);
        }
        else
        {
          return jank_nil;
        }
      },
      s,
      key);
  }

  native_bool contains(object_ref const s, object_ref const key)
  {
    if(s.is_nil())
    {
      return false;
    }

    return visit_object(
      [&](auto const typed_s) -> native_bool {
        using S = typename decltype(typed_s)::value_type;

        if constexpr(behavior::associatively_readable<S> || behavior::set_like<S>)
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

  object_ref merge(object_ref const m, object_ref const other)
  {
    return visit_object(
      [&](auto const typed_m) -> object_ref {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::associatively_writable<T>)
        {
          return visit_object(
            [&](auto const typed_other) -> object_ref {
              using O = typename decltype(typed_other)::value_type;

              if constexpr(std::same_as<O, obj::persistent_hash_map>
                           || std::same_as<O, obj::persistent_array_map>
                           || std::same_as<O, obj::transient_hash_map>
                           //|| std::same_as<O, obj::transient_array_map>
              )
              {
                object_ref ret{ typed_m };
                for(auto const &pair : typed_other->data)
                {
                  ret = assoc(ret, pair.first, pair.second);
                }
                return ret;
              }
              else if(std::same_as<O, obj::nil>)
              {
                return typed_m;
              }
              else
              {
                throw std::runtime_error{ util::format("not associatively readable: {} [{}]",
                                                       typed_other->to_code_string(),
                                                       object_type_str(typed_other->base.type)) };
              }
            },
            other);
        }
        else
        {
          throw std::runtime_error{ util::format("not associatively writable: {}",
                                                 typed_m->to_code_string()) };
        }
      },
      m);
  }

  object_ref subvec(object_ref const o, i64 const start, i64 const end)
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

  object_ref nth(object_ref const o, object_ref const idx)
  {
    auto const index(to_int(idx));
    if(index < 0)
    {
      throw std::runtime_error{ util::format("index out of bounds: {}", index) };
    }
    else if(o == jank_nil)
    {
      return o;
    }

    return visit_object(
      [&](auto const typed_o) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::indexable<T>)
        {
          return typed_o->nth(idx);
        }
        else if constexpr(behavior::seqable<T> && behavior::sequential<T>)
        {
          i64 i{};
          for(auto const e : make_sequence_range(typed_o))
          {
            if(i == index)
            {
              return e;
            }
            ++i;
          }
          throw std::runtime_error{ util::format("index out of bounds: {}", index) };
        }
        else
        {
          throw std::runtime_error{ util::format("not indexable: {}", object_type_str(o->type)) };
        }
      },
      o);
  }

  object_ref nth(object_ref const o, object_ref const idx, object_ref const fallback)
  {
    auto const index(to_int(idx));
    if(index < 0 || o == jank_nil)
    {
      return fallback;
    }

    return visit_object(
      [&](auto const typed_o) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::indexable<T>)
        {
          return typed_o->nth(idx, fallback);
        }
        else if constexpr(behavior::seqable<T> && behavior::sequential<T>)
        {
          i64 i{};
          for(auto const e : make_sequence_range(typed_o))
          {
            if(i == index)
            {
              return e;
            }
            ++i;
          }
          return fallback;
        }
        else
        {
          throw std::runtime_error{ util::format("not indexable: {}", object_type_str(o->type)) };
        }
      },
      o);
  }

  object_ref peek(object_ref const o)
  {
    if(o == jank_nil)
    {
      return o;
    }

    return visit_object(
      [&](auto const typed_o) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::stackable<T>)
        {
          return typed_o->peek();
        }
        else
        {
          throw std::runtime_error{ util::format("not stackable: {}", object_type_str(o->type)) };
        }
      },
      o);
  }

  object_ref pop(object_ref const o)
  {
    if(o == jank_nil)
    {
      return o;
    }

    return visit_object(
      [&](auto const typed_o) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::stackable<T>)
        {
          return typed_o->pop();
        }
        else
        {
          throw std::runtime_error{ util::format("not stackable: {}", object_type_str(o->type)) };
        }
      },
      o);
  }

  object_ref empty(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::collection_like<T>)
        {
          return T::empty();
        }
        else
        {
          return jank_nil;
        }
      },
      o);
  }

  jtl::immutable_string str(object_ref const o, object_ref const args)
  {
    util::string_builder buff;
    buff.reserve(16);
    if(!is_nil(o))
    {
      runtime::to_string(o, buff);
    }
    return visit_seqable(
      [](auto const typed_args, util::string_builder &buff) -> jtl::immutable_string {
        for(auto const e : make_sequence_range(typed_args))
        {
          if(is_nil(e))
          {
            continue;
          }
          runtime::to_string(e.erase(), buff);
        }
        return buff.release();
      },
      args,
      buff);
  }

  obj::persistent_list_ref list(object_ref const s)
  {
    return visit_seqable(
      [](auto const typed_s) -> obj::persistent_list_ref {
        return obj::persistent_list::create(typed_s);
      },
      s);
  }

  obj::persistent_vector_ref vec(object_ref const s)
  {
    return visit_seqable(
      [](auto const typed_s) -> obj::persistent_vector_ref {
        return obj::persistent_vector::create(typed_s);
      },
      s);
  }

  size_t sequence_length(object_ref const s)
  {
    return sequence_length(s, std::numeric_limits<size_t>::max());
  }

  size_t sequence_length(object_ref const s, size_t const max)
  {
    if(s.is_nil())
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
          auto const r{ make_sequence_range(typed_s) };
          for(auto i(r.begin()); i != r.end() && length < max; ++i)
          {
            ++length;
          }
          return length;
        }
        else
        {
          throw std::runtime_error{ util::format("not seqable: {}", typed_s->to_code_string()) };
        }
      },
      s);
  }

  native_bool sequence_equal(object_ref const l, object_ref const r)
  {
    if(l == r)
    {
      return true;
    }

    /* TODO: visit_sequence. */
    return visit_seqable(
      [](auto const typed_l, object_ref const r) -> native_bool {
        return visit_seqable(
          [](auto const typed_r, auto const typed_l) -> native_bool {
            auto const l_range{ make_sequence_range(typed_l) };
            auto const r_range{ make_sequence_range(typed_r) };
            auto r_it(r_range.begin());
            for(auto l_it(l_range.begin()); l_it != l_range.end(); ++l_it, ++r_it)
            {
              if(r_it == r_range.end() || !runtime::equal((*l_it).erase(), (*r_it).erase()))
              {
                return false;
              }
            }
            return r_it == r_range.end();
          },
          []() { return false; },
          r,
          typed_l);
      },
      []() { return false; },
      l,
      r);
  }

  object_ref reduce(object_ref const f, object_ref const init, object_ref const s)
  {
    return visit_seqable(
      [](auto const typed_coll, object_ref const f, object_ref const init) -> object_ref {
        object_ref res{ init };
        for(auto const e : make_sequence_range(typed_coll))
        {
          res = dynamic_call(f, res, e);
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

  object_ref reduced(object_ref const o)
  {
    return make_box<obj::reduced>(o);
  }

  native_bool is_reduced(object_ref const o)
  {
    return o->type == object_type::reduced;
  }

  object_ref chunk_buffer(object_ref const capacity)
  {
    return make_box<obj::chunk_buffer>(capacity);
  }

  object_ref chunk_append(object_ref const buff, object_ref const val)
  {
    auto const buffer(try_object<obj::chunk_buffer>(buff));
    buffer->append(val);
    return jank_nil;
  }

  object_ref chunk(object_ref const buff)
  {
    auto const buffer(try_object<obj::chunk_buffer>(buff));
    return buffer->chunk();
  }

  object_ref chunk_first(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::chunkable<T>)
        {
          return typed_o->chunked_first();
        }
        {
          throw std::runtime_error{ util::format("not chunkable: {}", typed_o->to_code_string()) };
        }
      },
      o);
  }

  object_ref chunk_next(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::chunkable<T>)
        {
          return typed_o->chunked_next();
        }
        {
          throw std::runtime_error{ util::format("not chunkable: {}", typed_o->to_code_string()) };
        }
      },
      o);
  }

  object_ref chunk_rest(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::chunkable<T>)
        {
          return typed_o->chunked_next().erase() ?: obj::persistent_list::empty().erase();
        }
        {
          throw std::runtime_error{ util::format("not chunkable: {}", typed_o->to_code_string()) };
        }
      },
      o);
  }

  object_ref chunk_cons(object_ref const chunk, object_ref const rest)
  {
    return make_box<obj::chunked_cons>(chunk, seq(rest));
  }

  native_bool is_chunked_seq(object_ref const o)
  {
    return visit_object(
      [=](auto const typed_o) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        return behavior::chunkable<T>;
      },
      o);
  }

  object_ref iterate(object_ref const fn, object_ref const o)
  {
    return make_box<obj::iterator>(fn, o);
  }

  object_ref repeat(object_ref const val)
  {
    return obj::repeat::create(val);
  }

  object_ref repeat(object_ref const n, object_ref const val)
  {
    return obj::repeat::create(n, val);
  }

  object_ref sort(object_ref const coll)
  {
    return visit_seqable(
      [](auto const typed_coll) -> object_ref {
        native_vector<object_ref> vec;
        for(auto const e : make_sequence_range(typed_coll))
        {
          vec.push_back(e);
        }

        std::stable_sort(vec.begin(), vec.end(), [](object_ref const a, object_ref const b) {
          return runtime::compare(a, b) < 0;
        });

        using T = typename decltype(typed_coll)::value_type;

        if constexpr(behavior::metadatable<T>)
        {
          return make_box<obj::native_vector_sequence>(typed_coll->meta, std::move(vec));
        }
        else
        {
          return make_box<obj::native_vector_sequence>(std::move(vec));
        }
      },
      coll);
  }

  object_ref shuffle(object_ref const coll)
  {
    return visit_seqable(
      [](auto const typed_coll) -> object_ref {
        native_vector<object_ref> vec;
        for(auto const e : make_sequence_range(typed_coll))
        {
          vec.push_back(e);
        }

        static std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(vec.begin(), vec.end(), g);

        return make_box<obj::persistent_vector>(
          runtime::detail::native_persistent_vector{ vec.begin(), vec.end() });
      },
      coll);
  }

}
