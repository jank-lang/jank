#pragma once

#include <prelude/object.hpp>

namespace jank
{
  struct sequence : virtual pool_item_common_base
  {
    using sequence_pointer = detail::box_type<sequence>;

    virtual object_ptr first() const = 0;
    virtual sequence_pointer next() const = 0;
  };
  using sequence_pointer = sequence::sequence_pointer;

  struct seqable
  { virtual sequence_pointer seq() const = 0; };

  struct string : object, pool_item_base<string>
  {
    string() = default;
    string(string &&) = default;
    string(string const &) = default;
    string(detail::string_type &&d)
      : data{ std::move(d) }
    { }

    detail::boolean_type equal(object const &) const override;
    detail::string_type to_string() const override;
    detail::integer_type to_hash() const override;

    string const* as_string() const override;

    detail::string_type data;
  };

  struct vector : object, seqable, pool_item_base<vector>
  {
    vector() = default;
    vector(vector &&) = default;
    vector(vector const &) = default;
    vector(detail::vector_type &&d)
      : data{ std::move(d) }
    { }
    vector(detail::vector_type const &d)
      : data{ d }
    { }

    detail::boolean_type equal(object const &) const override;
    detail::string_type to_string() const override;
    detail::integer_type to_hash() const override;

    vector const* as_vector() const override;
    seqable const* as_seqable() const override;

    sequence_pointer seq() const override;

    detail::vector_type data;
  };

  template <typename... Args>
  object_ptr JANK_VECTOR(Args &&... args)
  { return make_box<vector>(detail::vector_type{ std::forward<Args>(args)... }); }

  struct map : object, seqable, pool_item_base<map>
  {
    map() = default;
    map(map &&) = default;
    map(map const &) = default;
    map(detail::map_type &&d)
      : data{ std::move(d) }
    { }
    map(detail::map_type const &d)
      : data{ d }
    { }

    detail::boolean_type equal(object const &) const override;
    detail::string_type to_string() const override;
    detail::integer_type to_hash() const override;

    map const* as_map() const override;
    seqable const* as_seqable() const override;

    sequence_pointer seq() const override;

    detail::map_type data;
  };

  inline detail::map_type&& JANK_MAP_IMPL(detail::map_type &&m)
  { return std::move(m); }
  template <typename K, typename V, typename... Rest>
  detail::map_type JANK_MAP_IMPL(detail::map_type &&m, K &&k, V &&v, Rest &&... rest)
  {
    m.insert_or_assign(std::forward<K>(k), std::forward<V>(v));
    return JANK_MAP_IMPL(std::move(m), std::forward<Rest>(rest)...);
  }
  template <typename... Kvs>
  object_ptr JANK_MAP(Kvs &&... kvs)
  {
    /* XXX: It's somehow consistently faster to use `m` and move it, rather than just return
     * the result of JANK_MAP_IMPL. */
    detail::map_type m{ JANK_MAP_IMPL(detail::map_type{}, std::forward<Kvs>(kvs)...) };
    return make_box<map>(std::move(m));
  }

  struct set : object, seqable, pool_item_base<set>
  {
    set() = default;
    set(set &&) = default;
    set(set const &) = default;
    set(detail::set_type &&d)
      : data{ std::move(d) }
    { }
    set(detail::set_type const &d)
      : data{ d }
    { }

    detail::boolean_type equal(object const &) const override;
    detail::string_type to_string() const override;
    detail::integer_type to_hash() const override;

    set const* as_set() const override;
    seqable const* as_seqable() const override;

    sequence_pointer seq() const override;

    detail::set_type data;
  };

  object_ptr mapv(object_ptr const &f, object_ptr const &seq);
  object_ptr reduce(object_ptr const &f, object_ptr const &initial, object_ptr const &seq);
  object_ptr partition_gen_minus_all(object_ptr const &n, object_ptr const &seq);
  object_ptr range(object_ptr const &start, object_ptr const &end);
  object_ptr reverse(object_ptr const &seq);
  object_ptr get(object_ptr const &o, object_ptr const &key);
  object_ptr conj(object_ptr const &o, object_ptr const &val);
  object_ptr assoc(object_ptr const &o, object_ptr const &key, object_ptr const &val);
}
