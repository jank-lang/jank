#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/transient_hash_map.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime::obj
{
  persistent_hash_map::persistent_hash_map(option<object_ptr> const &meta,
                                           runtime::detail::native_persistent_array_map const &m,
                                           object_ptr const key,
                                           object_ptr const val)
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

  persistent_hash_map::persistent_hash_map(object_ptr const meta, value_type &&d)
    : data{ std::move(d) }
  {
    this->meta = meta;
  }

  persistent_hash_map::persistent_hash_map(option<object_ptr> const &meta, value_type &&d)
    : parent_type{ meta }
    , data{ std::move(d) }
  {
  }

  persistent_hash_map_ptr persistent_hash_map::create_from_seq(object_ptr const seq)
  {
    return make_box<persistent_hash_map>(visit_seqable(
      [](auto const typed_seq) -> persistent_hash_map::value_type {
        runtime::detail::native_transient_hash_map transient;
        for(auto it(typed_seq->fresh_seq()); it != nullptr; it = runtime::next_in_place(it))
        {
          auto const key(it->first());
          it = runtime::next_in_place(it);
          if(!it)
          {
            throw std::runtime_error{ fmt::format("Odd number of elements: {}",
                                                  typed_seq->to_string()) };
          }
          auto const val(it->first());
          transient.set(key, val);
        }
        return transient.persistent();
      },
      [=]() -> persistent_hash_map::value_type {
        throw std::runtime_error{ fmt::format("Not seqable: {}", runtime::to_string(seq)) };
      },
      seq));
  }

  object_ptr persistent_hash_map::get(object_ptr const key) const
  {
    auto const res(data.find(key));
    if(res)
    {
      return *res;
    }
    return nil::nil_const();
  }

  object_ptr persistent_hash_map::get(object_ptr const key, object_ptr const fallback) const
  {
    auto const res(data.find(key));
    if(res)
    {
      return *res;
    }
    return fallback;
  }

  object_ptr persistent_hash_map::get_entry(object_ptr const key) const
  {
    auto const res(data.find(key));
    if(res)
    {
      return make_box<persistent_vector>(std::in_place, key, *res);
    }
    return nil::nil_const();
  }

  native_bool persistent_hash_map::contains(object_ptr const key) const
  {
    return data.find(key);
  }

  persistent_hash_map_ptr
  persistent_hash_map::assoc(object_ptr const key, object_ptr const val) const
  {
    auto copy(data.set(key, val));
    return make_box<persistent_hash_map>(meta, std::move(copy));
  }

  persistent_hash_map_ptr persistent_hash_map::dissoc(object_ptr const key) const
  {
    auto copy(data.erase(key));
    return make_box<persistent_hash_map>(meta, std::move(copy));
  }

  object_ptr persistent_hash_map::call(object_ptr const o) const
  {
    return get(o);
  }

  object_ptr persistent_hash_map::call(object_ptr const o, object_ptr const fallback) const
  {
    return get(o, fallback);
  }

  transient_hash_map_ptr persistent_hash_map::to_transient() const
  {
    return make_box<transient_hash_map>(data);
  }
}
