#include <jank/runtime/obj/persistent_hash_set.hpp>

namespace jank::runtime
{
  obj::persistent_hash_set::static_object(runtime::detail::native_persistent_hash_set &&d)
    : data{ std::move(d) }
  {
  }

  obj::persistent_hash_set::static_object(runtime::detail::native_persistent_hash_set const &d)
    : data{ d }
  {
  }

  obj::persistent_hash_set::static_object(object_ptr const meta,
                                          runtime::detail::native_persistent_hash_set &&d)
    : data{ std::move(d) }
    , meta{ meta }
  {
  }

  obj::persistent_hash_set_ptr obj::persistent_hash_set::create_from_seq(object_ptr const seq)
  {
    return make_box<obj::persistent_hash_set>(visit_object(
      [](auto const typed_seq) -> obj::persistent_hash_set::value_type {
        using T = typename decltype(typed_seq)::value_type;

        if constexpr(behavior::seqable<T>)
        {
          detail::native_transient_hash_set transient;
          for(auto it(typed_seq->fresh_seq()); it != nullptr; it = runtime::next_in_place(it))
          {
            transient.insert(it->first());
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

  native_bool obj::persistent_hash_set::equal(object const &o) const
  {
    return runtime::equal(o, data.begin(), data.end());
  }

  void obj::persistent_hash_set::to_string(fmt::memory_buffer &buff) const
  {
    return runtime::to_string(data.begin(), data.end(), "#{", '}', buff);
  }

  native_persistent_string obj::persistent_hash_set::to_string() const
  {
    fmt::memory_buffer buff;
    runtime::to_string(data.begin(), data.end(), "#{", '}', buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_persistent_string obj::persistent_hash_set::to_code_string() const
  {
    fmt::memory_buffer buff;
    runtime::to_code_string(data.begin(), data.end(), "#{", '}', buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  /* TODO: Cache this. */
  native_hash obj::persistent_hash_set::to_hash() const
  {
    return hash::unordered(data.begin(), data.end());
  }

  obj::persistent_hash_set_sequence_ptr obj::persistent_hash_set::seq() const
  {
    return fresh_seq();
  }

  obj::persistent_hash_set_sequence_ptr obj::persistent_hash_set::fresh_seq() const
  {
    if(data.empty())
    {
      return nullptr;
    }
    return make_box<obj::persistent_hash_set_sequence>(this, data.begin(), data.end(), data.size());
  }

  size_t obj::persistent_hash_set::count() const
  {
    return data.size();
  }

  obj::persistent_hash_set_ptr obj::persistent_hash_set::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<obj::persistent_hash_set>(data));
    ret->meta = meta;
    return ret;
  }

  obj::persistent_hash_set_ptr obj::persistent_hash_set::conj(object_ptr const head) const
  {
    auto set(data.insert(head));
    auto ret(make_box<obj::persistent_hash_set>(std::move(set)));
    return ret;
  }

  object_ptr obj::persistent_hash_set::call(object_ptr const o)
  {
    auto const found(data.find(o));
    if(!found)
    {
      return obj::nil::nil_const();
    }
    return *found;
  }

  obj::transient_hash_set_ptr obj::persistent_hash_set::to_transient() const
  {
    return make_box<obj::transient_hash_set>(data);
  }

  native_bool obj::persistent_hash_set::contains(object_ptr const o) const
  {
    return data.find(o);
  }

  obj::persistent_hash_set_ptr obj::persistent_hash_set::disj(object_ptr const o) const
  {
    auto set(data.erase(o));
    auto ret(make_box<obj::persistent_hash_set>(std::move(set)));
    return ret;
  }
}
