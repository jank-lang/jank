#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/transient_hash_map.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  persistent_hash_map::persistent_hash_map(jtl::option<object_ref> const &meta,
                                           runtime::detail::native_persistent_array_map const &m,
                                           object_ref const key,
                                           object_ref const val)
    : parent_type{ meta }
  {
    runtime::detail::native_transient_hash_map transient;
    for(auto const &e : m)
    {
      transient.set(e.first, e.second);
    }
    transient.set(key, val);
    data = transient.persistent();
  }

  persistent_hash_map::persistent_hash_map(value_type &&d)
    : data{ std::move(d) }
  {
  }

  persistent_hash_map::persistent_hash_map(value_type const &d)
    : data{ d }
  {
  }

  persistent_hash_map::persistent_hash_map(jtl::option<object_ref> const &meta, value_type &&d)
    : parent_type{ meta }
    , data{ std::move(d) }
  {
  }

  persistent_hash_map_ref persistent_hash_map::create_from_seq(object_ref const seq)
  {
    return make_box<persistent_hash_map>(visit_seqable(
      [](auto const typed_seq) -> persistent_hash_map::value_type {
        runtime::detail::native_transient_hash_map transient;
        auto const r{ make_sequence_range(typed_seq) };
        for(auto it(r.begin()); it != r.end(); ++it)
        {
          auto const key(*it);
          ++it;
          if(it == r.end())
          {
            throw std::runtime_error{ util::format("Odd number of elements: {}",
                                                   typed_seq->to_string()) };
          }
          auto const val(*it);
          transient.set(key, val);
        }
        return transient.persistent();
      },
      [=]() -> persistent_hash_map::value_type {
        throw std::runtime_error{ util::format("Not seqable: {}", runtime::to_string(seq)) };
      },
      seq));
  }

  object_ref persistent_hash_map::get(object_ref const key) const
  {
    auto const res(data.find(key));
    if(res)
    {
      return *res;
    }
    return nil::nil_const();
  }

  object_ref persistent_hash_map::get(object_ref const key, object_ref const fallback) const
  {
    auto const res(data.find(key));
    if(res)
    {
      return *res;
    }
    return fallback;
  }

  object_ref persistent_hash_map::get_entry(object_ref const key) const
  {
    auto const res(data.find(key));
    if(res)
    {
      return make_box<persistent_vector>(std::in_place, key, *res);
    }
    return nil::nil_const();
  }

  native_bool persistent_hash_map::contains(object_ref const key) const
  {
    return data.find(key);
  }

  persistent_hash_map_ref
  persistent_hash_map::assoc(object_ref const key, object_ref const val) const
  {
    auto copy(data.set(key, val));
    return make_box<persistent_hash_map>(meta, std::move(copy));
  }

  persistent_hash_map_ref persistent_hash_map::dissoc(object_ref const key) const
  {
    auto copy(data.erase(key));
    return make_box<persistent_hash_map>(meta, std::move(copy));
  }

  persistent_hash_map_ref persistent_hash_map::conj(object_ref const head) const
  {
    if(head->type == object_type::persistent_array_map
       || head->type == object_type::persistent_hash_map)
    {
      return expect_object<persistent_hash_map>(runtime::merge(this, head));
    }

    if(head->type != object_type::persistent_vector)
    {
      throw std::runtime_error{ util::format("invalid map entry: {}", runtime::to_string(head)) };
    }

    auto const vec(expect_object<persistent_vector>(head));
    if(vec->count() != 2)
    {
      throw std::runtime_error{ util::format("invalid map entry: {}", runtime::to_string(head)) };
    }

    auto copy(data.set(vec->data[0], vec->data[1]));
    return make_box<persistent_hash_map>(meta, std::move(copy));
  }

  object_ref persistent_hash_map::call(object_ref const o) const
  {
    return get(o);
  }

  object_ref persistent_hash_map::call(object_ref const o, object_ref const fallback) const
  {
    return get(o, fallback);
  }

  transient_hash_map_ref persistent_hash_map::to_transient() const
  {
    return make_box<transient_hash_map>(data);
  }
}
