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
  auto seq(jtl::ref<T> const s)
  {
    return s->seq();
  }

  object_ref seq(object_ref s);

  template <typename T>
  requires behavior::seqable<T>
  auto fresh_seq(jtl::ref<T> const s)
  {
    return s->fresh_seq();
  }

  object_ref fresh_seq(object_ref s);

  template <typename T>
  requires behavior::sequenceable<T>
  auto next(jtl::ref<T> const s)
  {
    return s->next();
  }

  object_ref next(object_ref s);

  template <typename T>
  requires behavior::sequenceable_in_place<T>
  auto next_in_place(jtl::ref<T> const s)
  {
    return s->next_in_place();
  }

  template <typename T>
  requires(behavior::sequenceable<T> && !behavior::sequenceable_in_place<T>)
  auto next_in_place(jtl::ref<T> const s)
  {
    /* Not all sequences can be updated in place. For those, just gracefully
     * do a normal next. */
    return s->next();
  }

  object_ref next_in_place(object_ref s);

  object_ref rest(object_ref s);

  template <typename T>
  requires behavior::sequenceable<T>
  auto first(jtl::ref<T> const s)
  {
    return s->first();
  }

  object_ref first(object_ref s);

  template <typename T>
  requires behavior::sequenceable<T>
  auto second(jtl::ref<T> const s)
  {
    return next(s)->first();
  }

  object_ref second(object_ref s);

  bool is_empty(object_ref o);
  bool is_seq(object_ref o);
  bool is_sequential(object_ref o);
  bool is_seqable(object_ref o);
  bool is_collection(object_ref o);
  bool is_list(object_ref o);
  bool is_vector(object_ref o);
  bool is_map(object_ref o);
  bool is_associative(object_ref o);
  bool is_set(object_ref o);
  bool is_counter(object_ref o);
  bool is_transientable(object_ref o);
  bool is_sorted(object_ref o);

  object_ref transient(object_ref o);
  object_ref persistent(object_ref o);
  object_ref conj_in_place(object_ref coll, object_ref o);
  object_ref disj_in_place(object_ref coll, object_ref o);

  template <typename T>
  requires behavior::associatively_writable_in_place<T>
  auto assoc_in_place(oref<T> const m, object_ref const k, object_ref const v)
  {
    return m->assoc_in_place(k, v);
  }

  object_ref assoc_in_place(object_ref coll, object_ref k, object_ref v);
  object_ref dissoc_in_place(object_ref coll, object_ref k);
  object_ref pop_in_place(object_ref coll);

  object_ref cons(object_ref head, object_ref tail);
  object_ref conj(object_ref s, object_ref o);
  object_ref disj(object_ref s, object_ref o);

  template <typename T>
  requires behavior::associatively_writable<T>
  auto assoc(oref<T> const m, object_ref const k, object_ref const v)
  {
    return m->assoc(k, v);
  }

  object_ref assoc(object_ref m, object_ref k, object_ref v);
  object_ref dissoc(object_ref m, object_ref k);
  object_ref get(object_ref m, object_ref key);
  object_ref get(object_ref m, object_ref key, object_ref fallback);
  object_ref get_in(object_ref m, object_ref keys);
  object_ref get_in(object_ref m, object_ref keys, object_ref fallback);
  object_ref find(object_ref s, object_ref key);
  bool contains(object_ref s, object_ref key);
  object_ref merge(object_ref m, object_ref other);
  object_ref merge_in_place(object_ref m, object_ref other);
  object_ref subvec(object_ref o, i64 start, i64 end);
  object_ref nth(object_ref o, object_ref idx);
  object_ref nth(object_ref o, object_ref idx, object_ref fallback);
  object_ref peek(object_ref o);
  object_ref pop(object_ref o);
  object_ref empty(object_ref o);

  jtl::immutable_string str(object_ref o, object_ref args);

  obj::persistent_list_ref list(object_ref s);
  obj::persistent_vector_ref vec(object_ref s);

  bool sequence_equal(object_ref l, object_ref r);

  usize sequence_length(object_ref const s);
  usize sequence_length(object_ref const s, usize const max);

  object_ref reduce(object_ref f, object_ref init, object_ref s);
  object_ref reduced(object_ref o);
  bool is_reduced(object_ref o);

  object_ref chunk_buffer(object_ref capacity);
  object_ref chunk_append(object_ref buff, object_ref val);
  object_ref chunk(object_ref buff);
  object_ref chunk_first(object_ref o);
  object_ref chunk_next(object_ref o);
  object_ref chunk_rest(object_ref o);
  object_ref chunk_cons(object_ref chunk, object_ref rest);
  bool is_chunked_seq(object_ref o);

  object_ref iterate(object_ref fn, object_ref o);

  object_ref repeat(object_ref val);
  object_ref repeat(object_ref n, object_ref val);

  object_ref sort(object_ref coll);

  object_ref shuffle(object_ref coll);
}
