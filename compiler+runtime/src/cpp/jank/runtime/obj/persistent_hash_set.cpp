#include <jank/runtime/obj/persistent_hash_set.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime::obj
{
  persistent_hash_set::persistent_hash_set(value_type &&d)
    : data{ std::move(d) }
  {
  }

  persistent_hash_set::persistent_hash_set(value_type const &d)
    : data{ d }
  {
  }

  persistent_hash_set::persistent_hash_set(object_ptr const meta, value_type &&d)
    : data{ std::move(d) }
    , meta{ meta }
  {
  }

  persistent_hash_set::persistent_hash_set(option<object_ptr> const &meta, value_type &&d)
    : data{ std::move(d) }
    , meta{ meta }
  {
  }

  persistent_hash_set_ptr persistent_hash_set::empty()
  {
    static auto const ret(make_box<persistent_hash_set>());
    return ret;
  }

  persistent_hash_set_ptr persistent_hash_set::create_from_seq(object_ptr const seq)
  {
    return make_box<persistent_hash_set>(visit_seqable(
      [](auto const typed_seq) -> persistent_hash_set::value_type {
        runtime::detail::native_transient_hash_set transient;
        for(auto it(typed_seq->fresh_seq()); it != nil::nil_const(); it = runtime::next_in_place(it))
        {
          assert(it);
          transient.insert(it->first());
        }
        return transient.persistent();
      },
      seq));
  }

  native_bool persistent_hash_set::equal(object const &o) const
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

  void persistent_hash_set::to_string(util::string_builder &buff) const
  {
    runtime::to_string(data.begin(), data.end(), "#{", '}', buff);
  }

  native_persistent_string persistent_hash_set::to_string() const
  {
    util::string_builder buff;
    runtime::to_string(data.begin(), data.end(), "#{", '}', buff);
    return buff.release();
  }

  native_persistent_string persistent_hash_set::to_code_string() const
  {
    util::string_builder buff;
    runtime::to_code_string(data.begin(), data.end(), "#{", '}', buff);
    return buff.release();
  }

  /* TODO: Cache this. */
  native_hash persistent_hash_set::to_hash() const
  {
    return hash::unordered(data.begin(), data.end());
  }

  persistent_hash_set_sequence_ptr persistent_hash_set::seq() const
  {
    return fresh_seq();
  }

  persistent_hash_set_sequence_ptr persistent_hash_set::fresh_seq() const
  {
    if(data.empty())
    {
      return nil::nil_const();
    }
    return make_box<persistent_hash_set_sequence>(this, data.begin(), data.end(), data.size());
  }

  size_t persistent_hash_set::count() const
  {
    return data.size();
  }

  persistent_hash_set_ptr persistent_hash_set::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<persistent_hash_set>(data));
    ret->meta = meta;
    return ret;
  }

  persistent_hash_set_ptr persistent_hash_set::conj(object_ptr const head) const
  {
    auto set(data.insert(head));
    auto ret(make_box<persistent_hash_set>(meta, std::move(set)));
    return ret;
  }

  object_ptr persistent_hash_set::call(object_ptr const o) const
  {
    auto const found(data.find(o));
    if(!found)
    {
      return nil::nil_const();
    }
    return *found;
  }

  transient_hash_set_ptr persistent_hash_set::to_transient() const
  {
    return make_box<transient_hash_set>(data);
  }

  native_bool persistent_hash_set::contains(object_ptr const o) const
  {
    return data.find(o);
  }

  persistent_hash_set_ptr persistent_hash_set::disj(object_ptr const o) const
  {
    auto set(data.erase(o));
    auto ret(make_box<persistent_hash_set>(meta, std::move(set)));
    return ret;
  }
}
