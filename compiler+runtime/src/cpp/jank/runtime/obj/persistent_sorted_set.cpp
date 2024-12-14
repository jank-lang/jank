#include <jank/runtime/obj/persistent_sorted_set.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime::obj
{
  persistent_sorted_set::persistent_sorted_set(runtime::detail::native_persistent_sorted_set &&d)
    : data{ std::move(d) }
  {
  }

  persistent_sorted_set::persistent_sorted_set(
    runtime::detail::native_persistent_sorted_set const &d)
    : data{ d }
  {
  }

  persistent_sorted_set::persistent_sorted_set(object_ptr const meta,
                                               runtime::detail::native_persistent_sorted_set &&d)
    : data{ std::move(d) }
    , meta{ meta }
  {
  }

  persistent_sorted_set_ptr persistent_sorted_set::create_from_seq(object_ptr const seq)
  {
    return make_box<persistent_sorted_set>(visit_seqable(
      [](auto const typed_seq) -> persistent_sorted_set::value_type {
        runtime::detail::native_transient_sorted_set transient;
        for(auto it(typed_seq->fresh_seq()); it != nullptr; it = runtime::next_in_place(it))
        {
          transient.insert_v(it->first());
        }
        return transient.persistent();
      },
      seq));
  }

  native_bool persistent_sorted_set::equal(object const &o) const
  {
    if(&o == &base)
    {
      return true;
    }

    return visit_set_like(
      [&](auto const typed_o) -> native_bool {
        if(typed_o->count() != count())
        {
          return false;
        }

        for(auto const entry : data)
        {
          if(!typed_o->contains(entry))
          {
            return false;
          }
        }

        return true;
      },
      []() { return false; },
      &o);
  }

  void persistent_sorted_set::to_string(fmt::memory_buffer &buff) const
  {
    runtime::to_string(data.begin(), data.end(), "#{", '}', buff);
  }

  native_persistent_string persistent_sorted_set::to_string() const
  {
    fmt::memory_buffer buff;
    runtime::to_string(data.begin(), data.end(), "#{", '}', buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_persistent_string persistent_sorted_set::to_code_string() const
  {
    fmt::memory_buffer buff;
    runtime::to_code_string(data.begin(), data.end(), "#{", '}', buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  /* TODO: Cache this. */
  native_hash persistent_sorted_set::to_hash() const
  {
    return hash::unordered(data.begin(), data.end());
  }

  persistent_sorted_set_sequence_ptr persistent_sorted_set::seq() const
  {
    return fresh_seq();
  }

  persistent_sorted_set_sequence_ptr persistent_sorted_set::fresh_seq() const
  {
    if(data.empty())
    {
      return nullptr;
    }
    return make_box<persistent_sorted_set_sequence>(this, data.begin(), data.end(), data.size());
  }

  size_t persistent_sorted_set::count() const
  {
    return data.size();
  }

  persistent_sorted_set_ptr persistent_sorted_set::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<persistent_sorted_set>(data));
    ret->meta = meta;
    return ret;
  }

  persistent_sorted_set_ptr persistent_sorted_set::conj(object_ptr const head) const
  {
    auto set(data.insert_v(head));
    auto ret(make_box<persistent_sorted_set>(std::move(set)));
    return ret;
  }

  object_ptr persistent_sorted_set::call(object_ptr const o)
  {
    auto const found(data.find(o));
    if(found != data.end())
    {
      return found.get();
    }
    return nil::nil_const();
  }

  transient_sorted_set_ptr persistent_sorted_set::to_transient() const
  {
    return make_box<transient_sorted_set>(data);
  }

  native_bool persistent_sorted_set::contains(object_ptr const o) const
  {
    return data.find(o) != data.end();
  }

  persistent_sorted_set_ptr persistent_sorted_set::disj(object_ptr const o) const
  {
    auto set(data.erase_key(o));
    auto ret(make_box<persistent_sorted_set>(std::move(set)));
    return ret;
  }
}
