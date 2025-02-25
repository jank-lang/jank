#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/obj/persistent_sorted_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/transient_sorted_map.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime::obj
{
  persistent_sorted_map::persistent_sorted_map(value_type &&d)
    : data{ std::move(d) }
  {
  }

  persistent_sorted_map::persistent_sorted_map(value_type const &d)
    : data{ d }
  {
  }

  persistent_sorted_map::persistent_sorted_map(object_ptr const meta, value_type &&d)
    : data{ std::move(d) }
  {
    this->meta = meta;
  }

  persistent_sorted_map::persistent_sorted_map(option<object_ptr> const &meta, value_type &&d)
    : parent_type{ meta }
    , data{ std::move(d) }
  {
  }

  persistent_sorted_map_ptr persistent_sorted_map::create_from_seq(object_ptr const seq)
  {
    return make_box<persistent_sorted_map>(visit_object(
      [](auto const typed_seq) -> persistent_sorted_map::value_type {
        using T = typename decltype(typed_seq)::value_type;

        if constexpr(behavior::seqable<T>)
        {
          runtime::detail::native_transient_sorted_map transient;
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
            transient.insert_or_assign(key, val);
          }
          return transient.persistent();
        }
        else
        {
          throw std::runtime_error{ fmt::format("Not seqable: {}", typed_seq->to_string()) };
        }
      },
      seq));
  }

  object_ptr persistent_sorted_map::get(object_ptr const key) const
  {
    auto const res(data.find(key));
    if(res != data.end())
    {
      return res->second;
    }
    return nil::nil_const();
  }

  object_ptr persistent_sorted_map::get(object_ptr const key, object_ptr const fallback) const
  {
    auto const res(data.find(key));
    if(res != data.end())
    {
      return res->second;
    }
    return fallback;
  }

  object_ptr persistent_sorted_map::get_entry(object_ptr const key) const
  {
    auto const res(data.find(key));
    if(res != data.end())
    {
      return make_box<persistent_vector>(std::in_place, key, res->second);
    }
    return nil::nil_const();
  }

  native_bool persistent_sorted_map::contains(object_ptr const key) const
  {
    return data.find(key) != data.end();
  }

  persistent_sorted_map_ptr
  persistent_sorted_map::assoc(object_ptr const key, object_ptr const val) const
  {
    auto copy(data.insert_or_assign(key, val));
    return make_box<persistent_sorted_map>(meta, std::move(copy));
  }

  persistent_sorted_map_ptr persistent_sorted_map::dissoc(object_ptr const key) const
  {
    auto copy(data.erase_key(key));
    return make_box<persistent_sorted_map>(meta, std::move(copy));
  }

  object_ptr persistent_sorted_map::call(object_ptr const o) const
  {
    return get(o);
  }

  object_ptr persistent_sorted_map::call(object_ptr const o, object_ptr const fallback) const
  {
    return get(o, fallback);
  }

  transient_sorted_map_ptr persistent_sorted_map::to_transient() const
  {
    return make_box<transient_sorted_map>(data);
  }
}
