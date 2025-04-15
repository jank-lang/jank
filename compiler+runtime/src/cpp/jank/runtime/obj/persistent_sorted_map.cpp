#include <jank/runtime/obj/persistent_sorted_map.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/transient_sorted_map.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/util/fmt.hpp>

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

  persistent_sorted_map::persistent_sorted_map(object_ref const meta, value_type &&d)
    : data{ std::move(d) }
  {
    this->meta = meta;
  }

  persistent_sorted_map::persistent_sorted_map(jtl::option<object_ref> const &meta, value_type &&d)
    : parent_type{ meta }
    , data{ std::move(d) }
  {
  }

  persistent_sorted_map_ref persistent_sorted_map::empty()
  {
    static auto const ret(make_box<persistent_sorted_map>());
    return ret;
  }

  persistent_sorted_map_ref persistent_sorted_map::create_from_seq(object_ref const seq)
  {
    return make_box<persistent_sorted_map>(visit_object(
      [](auto const typed_seq) -> persistent_sorted_map::value_type {
        using T = typename decltype(typed_seq)::value_type;

        if constexpr(behavior::seqable<T>)
        {
          runtime::detail::native_transient_sorted_map transient;
          for(auto it(typed_seq->fresh_seq()); it.is_some(); it = it->next_in_place())
          {
            auto const key(it->first());
            it = it->next_in_place();
            if(it.is_nil())
            {
              throw std::runtime_error{ util::format("Odd number of elements: {}",
                                                     typed_seq->to_string()) };
            }
            auto const val(it->first());
            transient.insert_or_assign(key, val);
          }
          return transient.persistent();
        }
        else
        {
          throw std::runtime_error{ util::format("Not seqable: {}", typed_seq->to_string()) };
        }
      },
      seq));
  }

  object_ref persistent_sorted_map::get(object_ref const key) const
  {
    auto const res(data.find(key));
    if(res != data.end())
    {
      return res->second;
    }
    return nil::nil_const();
  }

  object_ref persistent_sorted_map::get(object_ref const key, object_ref const fallback) const
  {
    auto const res(data.find(key));
    if(res != data.end())
    {
      return res->second;
    }
    return fallback;
  }

  object_ref persistent_sorted_map::get_entry(object_ref const key) const
  {
    auto const res(data.find(key));
    if(res != data.end())
    {
      return make_box<persistent_vector>(std::in_place, key, res->second);
    }
    return nil::nil_const();
  }

  native_bool persistent_sorted_map::contains(object_ref const key) const
  {
    return data.find(key) != data.end();
  }

  persistent_sorted_map_ref
  persistent_sorted_map::assoc(object_ref const key, object_ref const val) const
  {
    auto copy(data.insert_or_assign(key, val));
    return make_box<persistent_sorted_map>(meta, std::move(copy));
  }

  persistent_sorted_map_ref persistent_sorted_map::dissoc(object_ref const key) const
  {
    auto copy(data.erase_key(key));
    return make_box<persistent_sorted_map>(meta, std::move(copy));
  }

  persistent_sorted_map_ref persistent_sorted_map::conj(object_ref const head) const
  {
    if(head->type == object_type::persistent_array_map
       || head->type == object_type::persistent_sorted_map)
    {
      return expect_object<persistent_sorted_map>(runtime::merge(this, head));
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

    auto copy(data.insert_or_assign(vec->data[0], vec->data[1]));
    return make_box<persistent_sorted_map>(meta, std::move(copy));
  }

  object_ref persistent_sorted_map::call(object_ref const o) const
  {
    return get(o);
  }

  object_ref persistent_sorted_map::call(object_ref const o, object_ref const fallback) const
  {
    return get(o, fallback);
  }

  transient_sorted_map_ref persistent_sorted_map::to_transient() const
  {
    return make_box<transient_sorted_map>(data);
  }
}
