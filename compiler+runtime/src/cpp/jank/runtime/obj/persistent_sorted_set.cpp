#include <jank/runtime/util.hpp>
#include <jank/runtime/obj/persistent_sorted_set.hpp>

namespace jank::runtime
{
  obj::persistent_sorted_set::static_object(runtime::detail::native_persistent_sorted_set &&d)
    : data{ std::move(d) }
  {
  }

  obj::persistent_sorted_set::static_object(runtime::detail::native_persistent_sorted_set const &d)
    : data{ d }
  {
  }

  obj::persistent_sorted_set::static_object(object_ptr const meta,
                                            runtime::detail::native_persistent_sorted_set &&d)
    : data{ std::move(d) }
    , meta{ meta }
  {
  }

  obj::persistent_sorted_set_ptr obj::persistent_sorted_set::create_from_seq(object_ptr const seq)
  {
    return make_box<obj::persistent_sorted_set>(visit_object(
      [](auto const typed_seq) -> obj::persistent_sorted_set::value_type {
        using T = typename decltype(typed_seq)::value_type;

        if constexpr(behavior::seqable<T>)
        {
          detail::native_transient_sorted_set transient;
          for(auto it(typed_seq->fresh_seq()); it != nullptr; it = runtime::next_in_place(it))
          {
            transient.insert_v(it->first());
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

  native_bool obj::persistent_sorted_set::equal(object const &o) const
  {
    return runtime::equal(o, data.begin(), data.end());
  }

  void obj::persistent_sorted_set::to_string(fmt::memory_buffer &buff) const
  {
    return runtime::to_string(data.begin(), data.end(), "#{", '}', buff);
  }

  native_persistent_string obj::persistent_sorted_set::to_string() const
  {
    fmt::memory_buffer buff;
    runtime::to_string(data.begin(), data.end(), "#{", '}', buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  /* TODO: Cache this. */
  native_hash obj::persistent_sorted_set::to_hash() const
  {
    return hash::unordered(data.begin(), data.end());
  }

  obj::persistent_sorted_set_sequence_ptr obj::persistent_sorted_set::seq() const
  {
    return fresh_seq();
  }

  obj::persistent_sorted_set_sequence_ptr obj::persistent_sorted_set::fresh_seq() const
  {
    if(data.empty())
    {
      return nullptr;
    }
    return make_box<obj::persistent_sorted_set_sequence>(this,
                                                         data.begin(),
                                                         data.end(),
                                                         data.size());
  }

  size_t obj::persistent_sorted_set::count() const
  {
    return data.size();
  }

  obj::persistent_sorted_set_ptr obj::persistent_sorted_set::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<obj::persistent_sorted_set>(data));
    ret->meta = meta;
    return ret;
  }

  obj::persistent_sorted_set_ptr obj::persistent_sorted_set::conj(object_ptr const head) const
  {
    auto set(data.insert_v(head));
    auto ret(make_box<obj::persistent_sorted_set>(std::move(set)));
    return ret;
  }

  object_ptr obj::persistent_sorted_set::call(object_ptr const o)
  {
    auto const found(data.find(o));
    if(found != data.end())
    {
      return found.get();
    }
    return obj::nil::nil_const();
  }

  obj::transient_sorted_set_ptr obj::persistent_sorted_set::to_transient() const
  {
    return make_box<obj::transient_sorted_set>(data);
  }

  native_bool obj::persistent_sorted_set::contains(object_ptr const o) const
  {
    return data.find(o) != data.end();
  }

  obj::persistent_sorted_set_ptr obj::persistent_sorted_set::disj(object_ptr const o) const
  {
    auto set(data.erase_key(o));
    auto ret(make_box<obj::persistent_sorted_set>(std::move(set)));
    return ret;
  }
}
