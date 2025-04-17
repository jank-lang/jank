#include <jank/runtime/obj/persistent_sorted_set.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime::obj
{
  persistent_sorted_set::persistent_sorted_set(value_type &&d)
    : data{ std::move(d) }
  {
  }

  persistent_sorted_set::persistent_sorted_set(
    runtime::detail::native_persistent_sorted_set const &d)
    : data{ d }
  {
  }

  persistent_sorted_set::persistent_sorted_set(object_ref const meta, value_type &&d)
    : data{ std::move(d) }
    , meta{ meta }
  {
  }

  persistent_sorted_set::persistent_sorted_set(jtl::option<object_ref> const &meta, value_type &&d)
    : data{ std::move(d) }
    , meta{ meta }
  {
  }

  persistent_sorted_set_ref persistent_sorted_set::empty()
  {
    static auto const ret(make_box<persistent_sorted_set>());
    return ret;
  }

  persistent_sorted_set_ref persistent_sorted_set::create_from_seq(object_ref const seq)
  {
    return make_box<persistent_sorted_set>(visit_seqable(
      [](auto const typed_seq) -> persistent_sorted_set::value_type {
        runtime::detail::native_transient_sorted_set transient;
        for(auto const e : make_sequence_range(typed_seq))
        {
          transient.insert_v(e);
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

  void persistent_sorted_set::to_string(util::string_builder &buff) const
  {
    runtime::to_string(data.begin(), data.end(), "#{", '}', buff);
  }

  jtl::immutable_string persistent_sorted_set::to_string() const
  {
    util::string_builder buff;
    runtime::to_string(data.begin(), data.end(), "#{", '}', buff);
    return buff.release();
  }

  jtl::immutable_string persistent_sorted_set::to_code_string() const
  {
    util::string_builder buff;
    runtime::to_code_string(data.begin(), data.end(), "#{", '}', buff);
    return buff.release();
  }

  /* TODO: Cache this. */
  native_hash persistent_sorted_set::to_hash() const
  {
    return hash::unordered(data.begin(), data.end());
  }

  persistent_sorted_set_sequence_ref persistent_sorted_set::seq() const
  {
    return fresh_seq();
  }

  persistent_sorted_set_sequence_ref persistent_sorted_set::fresh_seq() const
  {
    if(data.empty())
    {
      return {};
    }
    return make_box<persistent_sorted_set_sequence>(this, data.begin(), data.end(), data.size());
  }

  size_t persistent_sorted_set::count() const
  {
    return data.size();
  }

  persistent_sorted_set_ref persistent_sorted_set::with_meta(object_ref const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<persistent_sorted_set>(data));
    ret->meta = meta;
    return ret;
  }

  persistent_sorted_set_ref persistent_sorted_set::conj(object_ref const head) const
  {
    auto set(data.insert_v(head));
    auto ret(make_box<persistent_sorted_set>(meta, std::move(set)));
    return ret;
  }

  object_ref persistent_sorted_set::call(object_ref const o)
  {
    auto const found(data.find(o));
    if(found != data.end())
    {
      return found.get();
    }
    return nil::nil_const();
  }

  transient_sorted_set_ref persistent_sorted_set::to_transient() const
  {
    return make_box<transient_sorted_set>(data);
  }

  native_bool persistent_sorted_set::contains(object_ref const o) const
  {
    return data.find(o) != data.end();
  }

  persistent_sorted_set_ref persistent_sorted_set::disj(object_ref const o) const
  {
    auto set(data.erase_key(o));
    auto ret(make_box<persistent_sorted_set>(meta, std::move(set)));
    return ret;
  }
}

namespace jank::runtime
{
  template obj::persistent_sorted_set_ref
  make_box<obj::persistent_sorted_set>(obj::persistent_sorted_set::value_type &&);
  template obj::persistent_sorted_set_ref
  make_box<obj::persistent_sorted_set>(obj::persistent_sorted_set::value_type const &);
}
