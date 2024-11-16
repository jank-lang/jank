#pragma once

#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/core/to_string.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_list = static_object<object_type::persistent_list>;
    using persistent_list_ptr = native_box<persistent_list>;
  }

  template <typename T>
  requires behavior::seqable<T>
  auto seq(native_box<T> const s)
  {
    return s->seq();
  }

  object_ptr seq(object_ptr s);

  template <typename T>
  requires behavior::seqable<T>
  auto fresh_seq(native_box<T> const s)
  {
    return s->fresh_seq();
  }

  object_ptr fresh_seq(object_ptr s);

  template <typename T>
  requires behavior::sequenceable<T>
  auto next(native_box<T> const s)
  {
    return s->next();
  }

  object_ptr next(object_ptr s);

  template <typename T>
  requires behavior::sequenceable_in_place<T>
  auto next_in_place(native_box<T> const s)
  {
    return s->next_in_place();
  }

  template <typename T>
  requires(behavior::sequenceable<T> && !behavior::sequenceable_in_place<T>)
  auto next_in_place(native_box<T> const s)
  {
    /* Not all sequences can be updated in place. For those, just gracefully
     * do a normal next. */
    return s->next();
  }

  object_ptr next_in_place(object_ptr s);

  /* TODO: core header post-erasure? */
  //template <typename T>
  //requires behavior::sequenceable<T>
  //auto rest(native_box<T> const seq)
  //{
  //  if(!seq || seq == obj::nil::nil_const())
  //  {
  //    return obj::persistent_list::empty();
  //  }
  //  auto const ret(seq->next());
  //  if(ret == nullptr)
  //  {
  //    return obj::persistent_list::empty();
  //  }
  //  return ret;
  //}

  object_ptr rest(object_ptr s);

  template <typename T>
  requires behavior::sequenceable<T>
  auto first(native_box<T> const s)
  {
    return s->first();
  }

  object_ptr first(object_ptr s);

  template <typename T>
  requires behavior::sequenceable<T>
  auto second(native_box<T> const s)
  {
    return next(s)->first();
  }

  object_ptr second(object_ptr s);

  native_bool is_empty(object_ptr o);
  native_bool is_seq(object_ptr o);
  native_bool is_sequential(object_ptr o);
  native_bool is_collection(object_ptr o);
  native_bool is_list(object_ptr o);
  native_bool is_vector(object_ptr o);
  native_bool is_map(object_ptr o);
  native_bool is_set(object_ptr o);
  native_bool is_transientable(object_ptr o);

  object_ptr transient(object_ptr o);
  object_ptr persistent(object_ptr o);
  object_ptr conj_in_place(object_ptr coll, object_ptr o);
  object_ptr disj_in_place(object_ptr coll, object_ptr o);
  object_ptr assoc_in_place(object_ptr coll, object_ptr k, object_ptr v);
  object_ptr dissoc_in_place(object_ptr coll, object_ptr k);
  object_ptr pop_in_place(object_ptr coll);

  object_ptr cons(object_ptr head, object_ptr tail);
  object_ptr conj(object_ptr s, object_ptr o);
  object_ptr disj(object_ptr s, object_ptr o);
  object_ptr assoc(object_ptr m, object_ptr k, object_ptr v);
  object_ptr dissoc(object_ptr m, object_ptr k);
  object_ptr get(object_ptr m, object_ptr key);
  object_ptr get(object_ptr m, object_ptr key, object_ptr fallback);
  object_ptr get_in(object_ptr m, object_ptr keys);
  object_ptr get_in(object_ptr m, object_ptr keys, object_ptr fallback);
  object_ptr find(object_ptr s, object_ptr key);
  native_bool contains(object_ptr s, object_ptr key);
  object_ptr merge(object_ptr m, object_ptr other);
  object_ptr subvec(object_ptr o, native_integer start, native_integer end);
  object_ptr nth(object_ptr o, object_ptr idx);
  object_ptr nth(object_ptr o, object_ptr idx, object_ptr fallback);
  object_ptr peek(object_ptr o);
  object_ptr pop(object_ptr o);
  object_ptr empty(object_ptr o);

  native_persistent_string str(object_ptr o, object_ptr args);

  obj::persistent_list_ptr list(object_ptr s);
  obj::persistent_vector_ptr vec(object_ptr s);

  template <typename It>
  native_bool equal(object const &o, It const begin, It const end)
  {
    return visit_seqable(
      [](auto const typed_o, auto const begin, auto const end) -> native_bool {
        using T = typename decltype(typed_o)::value_type;

        /* nil is seqable, but we don't want it to be equal to an empty collection.
           An empty seq itself is nil, but that's different. */
        if constexpr(std::same_as<T, obj::nil>)
        {
          return false;
        }
        else
        {
          auto seq(typed_o->fresh_seq());
          auto it(begin);
          for(; it != end; ++it, seq = runtime::next_in_place(seq))
          {
            if(seq == nullptr || !runtime::equal(*it, seq->first()))
            {
              return false;
            }
          }
          return seq == nullptr && it == end;
        }
      },
      []() { return false; },
      &o,
      begin,
      end);
  }

  native_bool sequence_equal(object_ptr l, object_ptr r);

  size_t sequence_length(object_ptr const s);
  size_t sequence_length(object_ptr const s, size_t const max);

  object_ptr reduce(object_ptr f, object_ptr init, object_ptr s);
  object_ptr reduced(object_ptr o);
  native_bool is_reduced(object_ptr o);

  object_ptr chunk_buffer(object_ptr capacity);
  object_ptr chunk_append(object_ptr buff, object_ptr val);
  object_ptr chunk(object_ptr buff);
  object_ptr chunk_first(object_ptr o);
  object_ptr chunk_next(object_ptr o);
  object_ptr chunk_rest(object_ptr o);
  object_ptr chunk_cons(object_ptr chunk, object_ptr rest);
  native_bool is_chunked_seq(object_ptr o);

  object_ptr iterate(object_ptr fn, object_ptr o);

  object_ptr repeat(object_ptr val);
  object_ptr repeat(object_ptr n, object_ptr val);
}
