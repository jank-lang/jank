#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/associatively_writable.hpp>
#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_list_ref = oref<struct persistent_list>;
    using persistent_vector_ref = oref<struct persistent_vector>;
  }

  template <typename T>
  requires behavior::seqable<T>
  auto seq(oref<T> const &s)
  {
    return s->seq();
  }

  object_ref seq(object_ref const s);

  template <typename T>
  requires behavior::seqable<T>
  auto fresh_seq(oref<T> const &s)
  {
    return s->fresh_seq();
  }

  object_ref fresh_seq(object_ref const s);

  template <typename T>
  requires behavior::sequenceable<T>
  auto next(oref<T> const &s)
  {
    return s->next();
  }

  object_ref next(object_ref const s);

  template <typename T>
  requires behavior::sequenceable_in_place<T>
  auto next_in_place(oref<T> const &s)
  {
    return s->next_in_place();
  }

  template <typename T>
  requires(behavior::sequenceable<T> && !behavior::sequenceable_in_place<T>)
  auto next_in_place(oref<T> const &s)
  {
    /* Not all sequences can be updated in place. For those, just gracefully
     * do a normal next. */
    return s->next();
  }

  object_ref next_in_place(object_ref const s);

  object_ref rest(object_ref const s);

  template <typename T>
  requires behavior::sequenceable<T>
  auto first(oref<T> const &s)
  {
    return s->first();
  }

  object_ref first(object_ref const s);

  template <typename T>
  requires behavior::sequenceable<T>
  auto second(oref<T> const &s)
  {
    return next(s)->first();
  }

  object_ref second(object_ref const s);

  bool is_empty(object_ref const o);
  bool is_seq(object_ref const o);
  bool is_sequential(object_ref const o);
  bool is_seqable(object_ref const o);
  bool is_collection(object_ref const o);
  bool is_list(object_ref const o);
  bool is_vector(object_ref const o);
  bool is_map(object_ref const o);
  bool is_associative(object_ref const o);
  bool is_set(object_ref const o);
  bool is_counted(object_ref const o);
  bool is_transientable(object_ref const o);
  bool is_sorted(object_ref const o);

  object_ref transient(object_ref const o);
  object_ref persistent(object_ref const o);
  object_ref conj_in_place(object_ref const coll, object_ref const o);
  object_ref disj_in_place(object_ref const coll, object_ref const o);

  template <typename T>
  requires behavior::associatively_writable_in_place<T>
  auto assoc_in_place(oref<T> const &m, object_ref const k, object_ref const v)
  {
    return m->assoc_in_place(k, v);
  }

  object_ref assoc_in_place(object_ref const coll, object_ref const k, object_ref const v);
  object_ref dissoc_in_place(object_ref const coll, object_ref const k);
  object_ref pop_in_place(object_ref const coll);

  object_ref cons(object_ref const head, object_ref const tail);
  object_ref conj(object_ref const s, object_ref const o);
  object_ref disj(object_ref const s, object_ref const o);

  template <typename T>
  requires behavior::associatively_writable<T>
  auto assoc(oref<T> const &m, object_ref const k, object_ref const v)
  {
    return m->assoc(k, v);
  }

  object_ref assoc(object_ref const m, object_ref const k, object_ref const v);
  object_ref dissoc(object_ref const m, object_ref const k);
  object_ref get(object_ref const m, object_ref const key);
  object_ref get(object_ref const m, object_ref const key, object_ref const fallback);
  object_ref get_in(object_ref const m, object_ref const keys);
  object_ref get_in(object_ref const m, object_ref const keys, object_ref const fallback);
  object_ref find(object_ref const s, object_ref const key);
  bool contains(object_ref const s, object_ref const key);
  object_ref merge(object_ref const m, object_ref const other);
  object_ref merge_in_place(object_ref const m, object_ref const other);
  object_ref subvec(object_ref const o, i64 start, i64 end);
  object_ref nth(object_ref const o, object_ref const idx);
  object_ref nth(object_ref const o, object_ref const idx, object_ref const fallback);
  object_ref peek(object_ref const o);
  object_ref pop(object_ref const o);
  object_ref empty(object_ref const o);

  jtl::immutable_string str(object_ref const o);
  jtl::immutable_string str(object_ref const o, object_ref const args);

  obj::persistent_list_ref list(object_ref const s);
  obj::persistent_vector_ref vec(object_ref const s);

  bool sequence_equal(object_ref const l, object_ref const r);

  usize sequence_length(object_ref const s);
  usize sequence_length(object_ref const s, usize const max);

  object_ref reduce(object_ref const f, object_ref const init, object_ref const s);
  object_ref reduced(object_ref const o);
  bool is_reduced(object_ref const o);

  object_ref chunk_buffer(object_ref const capacity);
  object_ref chunk_append(object_ref const buff, object_ref const val);
  object_ref chunk(object_ref const buff);
  object_ref chunk_first(object_ref const o);
  object_ref chunk_next(object_ref const o);
  object_ref chunk_rest(object_ref const o);
  object_ref chunk_cons(object_ref const chunk, object_ref const rest);
  bool is_chunked_seq(object_ref const o);

  object_ref iterate(object_ref const fn, object_ref const o);

  object_ref repeat(object_ref const val);
  object_ref repeat(object_ref const n, object_ref const val);

  object_ref sort(object_ref const coll);

  object_ref shuffle(object_ref const coll);
}
