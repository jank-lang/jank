#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/transient_vector.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/seq_ext.hpp>
#include <jank/runtime/behavior/sequential.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  persistent_vector::persistent_vector(value_type &&d)
    : data{ std::move(d) }
  {
  }

  persistent_vector::persistent_vector(value_type const &d)
    : data{ d }
  {
  }

  persistent_vector::persistent_vector(jtl::option<object_ref> const &meta, value_type &&d)
    : data{ std::move(d) }
    , meta{ meta }
  {
  }

  persistent_vector_ref persistent_vector::create(object_ref const s)
  {
    if(s.is_nil())
    {
      return make_box<persistent_vector>();
    }

    return visit_object(
      [](auto const typed_s) -> persistent_vector_ref {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          runtime::detail::native_transient_vector v;
          for(auto const e : make_sequence_range(typed_s))
          {
            v.push_back(e);
          }
          return make_box<persistent_vector>(v.persistent());
        }
        else
        {
          throw std::runtime_error{ util::format("invalid sequence: {}", typed_s->to_string()) };
        }
      },
      s);
  }

  persistent_vector_ref persistent_vector::empty()
  {
    static auto const ret(make_box<persistent_vector>());
    return ret;
  }

  native_bool persistent_vector::equal(object const &o) const
  {
    if(&o == &base)
    {
      return true;
    }

    /* TODO: Optimize this to use iterators. They're faster for immer vectors. */
    auto const v{ dyn_cast<persistent_vector>(&o) };
    if(v.is_some())
    {
      if(data.size() != v->data.size())
      {
        return false;
      }
      for(usize i{}; i < data.size(); ++i)
      {
        if(!runtime::equal(data[i], v->data[i]))
        {
          return false;
        }
      }
      return true;
    }
    else
    {
      return runtime::sequence_equal(this, &o);
    }
  }

  void persistent_vector::to_string(util::string_builder &buff) const
  {
    runtime::to_string(data.begin(), data.end(), "[", ']', buff);
  }

  jtl::immutable_string persistent_vector::to_string() const
  {
    util::string_builder buff;
    runtime::to_string(data.begin(), data.end(), "[", ']', buff);
    return buff.release();
  }

  jtl::immutable_string persistent_vector::to_code_string() const
  {
    util::string_builder buff;
    runtime::to_code_string(data.begin(), data.end(), "[", ']', buff);
    return buff.release();
  }

  native_hash persistent_vector::to_hash() const
  {
    if(hash != 0)
    {
      return hash;
    }

    return hash = hash::ordered(data.begin(), data.end());
  }

  i64 persistent_vector::compare(object const &o) const
  {
    return visit_type<persistent_vector>([this](auto const typed_o) { return compare(*typed_o); },
                                         &o);
  }

  i64 persistent_vector::compare(persistent_vector const &v) const
  {
    auto const size(data.size());
    auto const v_size(v.data.size());

    if(size < v_size)
    {
      return -1;
    }
    if(size > v_size)
    {
      return 1;
    }

    for(usize i{}; i < size; ++i)
    {
      auto const res(runtime::compare(data[i], v.data[i]));
      if(res != 0)
      {
        return res;
      }
    }

    return 0;
  }

  persistent_vector_sequence_ref persistent_vector::seq() const
  {
    return fresh_seq();
  }

  persistent_vector_sequence_ref persistent_vector::fresh_seq() const
  {
    if(data.empty())
    {
      return {};
    }
    return make_box<persistent_vector_sequence>(const_cast<persistent_vector *>(this));
  }

  usize persistent_vector::count() const
  {
    return data.size();
  }

  persistent_vector_ref persistent_vector::conj(object_ref head) const
  {
    auto vec(data.push_back(head));
    auto ret(make_box<persistent_vector>(meta, std::move(vec)));
    return ret;
  }

  transient_vector_ref persistent_vector::to_transient() const
  {
    return make_box<transient_vector>(data);
  }

  persistent_vector_ref persistent_vector::with_meta(object_ref const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<persistent_vector>(data));
    ret->meta = meta;
    return ret;
  }

  object_ref persistent_vector::get(object_ref const key) const
  {
    return get(key, jank_nil);
  }

  object_ref persistent_vector::get(object_ref const key, object_ref const fallback) const
  {
    if(key->type == object_type::integer)
    {
      auto const i(expect_object<integer>(key)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return fallback;
      }
      return data[i];
    }
    else
    {
      return fallback;
    }
  }

  object_ref persistent_vector::get_entry(object_ref const key) const
  {
    if(key->type == object_type::integer)
    {
      auto const i(expect_object<integer>(key)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return jank_nil;
      }
      /* TODO: Map entry type? */
      return make_box<persistent_vector>(std::in_place, key, data[i]);
    }
    else
    {
      return jank_nil;
    }
  }

  native_bool persistent_vector::contains(object_ref const key) const
  {
    if(key->type == object_type::integer)
    {
      auto const i(expect_object<integer>(key)->data);
      return i >= 0 && static_cast<size_t>(i) < data.size();
    }
    else
    {
      return false;
    }
  }

  object_ref persistent_vector::peek() const
  {
    if(data.empty())
    {
      return jank_nil;
    }

    return data[data.size() - 1];
  }

  persistent_vector_ref persistent_vector::pop() const
  {
    if(data.empty())
    {
      throw std::runtime_error{ "cannot pop an empty vector" };
    }

    return make_box<persistent_vector>(meta, data.take(data.size() - 1));
  }

  object_ref persistent_vector::nth(object_ref const index) const
  {
    if(index->type == object_type::integer)
    {
      auto const i(expect_object<integer>(index)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        throw std::runtime_error{
          util::format("out of bounds index {}; vector has a size of {}", i, data.size())
        };
      }
      return data[i];
    }
    else
    {
      throw std::runtime_error{ util::format("nth on a vector must be an integer; found {}",
                                             runtime::to_string(index)) };
    }
  }

  object_ref persistent_vector::nth(object_ref const index, object_ref const fallback) const
  {
    return get(index, fallback);
  }
}
