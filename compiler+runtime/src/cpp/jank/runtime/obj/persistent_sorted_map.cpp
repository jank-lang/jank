#include <jank/runtime/obj/persistent_sorted_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/transient_sorted_map.hpp>

namespace jank::runtime
{
  obj::persistent_sorted_map::static_object(value_type &&d)
    : data{ std::move(d) }
  {
  }

  obj::persistent_sorted_map::static_object(value_type const &d)
    : data{ d }
  {
  }

  obj::persistent_sorted_map::static_object(object_ptr const meta, value_type &&d)
    : data{ std::move(d) }
  {
    this->meta = meta;
  }

  obj::persistent_sorted_map_ptr obj::persistent_sorted_map::create_from_seq(object_ptr const seq)
  {
    return make_box<obj::persistent_sorted_map>(visit_object(
      [](auto const typed_seq) -> obj::persistent_sorted_map::value_type {
        using T = typename decltype(typed_seq)::value_type;

        if constexpr(behavior::seqable<T>)
        {
          detail::native_transient_sorted_map transient;
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

  object_ptr obj::persistent_sorted_map::get(object_ptr const key) const
  {
    auto const res(data.find(key));
    if(res != data.end())
    {
      return res->second;
    }
    return obj::nil::nil_const();
  }

  object_ptr obj::persistent_sorted_map::get(object_ptr const key, object_ptr const fallback) const
  {
    auto const res(data.find(key));
    if(res != data.end())
    {
      return res->second;
    }
    return fallback;
  }

  object_ptr obj::persistent_sorted_map::get_entry(object_ptr const key) const
  {
    auto const res(data.find(key));
    if(res != data.end())
    {
      return make_box<obj::persistent_vector>(std::in_place, key, res->second);
    }
    return obj::nil::nil_const();
  }

  native_bool obj::persistent_sorted_map::contains(object_ptr const key) const
  {
    return data.find(key) != data.end();
  }

  obj::persistent_sorted_map_ptr
  obj::persistent_sorted_map::assoc(object_ptr const key, object_ptr const val) const
  {
    auto copy(data.insert_or_assign(key, val));
    return make_box<obj::persistent_sorted_map>(std::move(copy));
  }

  obj::persistent_sorted_map_ptr obj::persistent_sorted_map::dissoc(object_ptr const key) const
  {
    auto copy(data.erase_key(key));
    return make_box<obj::persistent_sorted_map>(std::move(copy));
  }

  obj::persistent_sorted_map_ptr obj::persistent_sorted_map::conj(object_ptr const head) const
  {
    if(head->type == object_type::persistent_array_map
       || head->type == object_type::persistent_sorted_map)
    {
      return expect_object<obj::persistent_sorted_map>(runtime::merge(this, head));
    }

    if(head->type != object_type::persistent_vector)
    {
      throw std::runtime_error{ fmt::format("invalid map entry: {}", runtime::to_string(head)) };
    }

    auto const vec(expect_object<obj::persistent_vector>(head));
    if(vec->count() != 2)
    {
      throw std::runtime_error{ fmt::format("invalid map entry: {}", runtime::to_string(head)) };
    }

    auto copy(data.insert_or_assign(vec->data[0], vec->data[1]));
    return make_box<obj::persistent_sorted_map>(std::move(copy));
  }

  object_ptr obj::persistent_sorted_map::call(object_ptr const o)
  {
    return get(o);
  }

  object_ptr obj::persistent_sorted_map::call(object_ptr const o, object_ptr const fallback)
  {
    return get(o, fallback);
  }

  obj::transient_sorted_map_ptr obj::persistent_sorted_map::to_transient() const
  {
    return make_box<obj::transient_sorted_map>(data);
  }
}
