#include <jank/runtime/util.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/transient_vector.hpp>

namespace jank::runtime
{
  obj::persistent_vector::static_object(runtime::detail::native_persistent_vector &&d)
    : data{ std::move(d) }
  {
  }

  obj::persistent_vector::static_object(runtime::detail::native_persistent_vector const &d)
    : data{ d }
  {
  }

  obj::persistent_vector::static_object(object_ptr const meta,
                                        runtime::detail::native_persistent_vector &&d)
    : data{ std::move(d) }
    , meta{ meta }
  {
  }

  obj::persistent_vector_ptr obj::persistent_vector::create(object_ptr const s)
  {
    if(s == nullptr)
    {
      return make_box<obj::persistent_vector>();
    }

    return visit_object(
      [](auto const typed_s) -> obj::persistent_vector_ptr {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          runtime::detail::native_transient_vector v;
          for(auto i(typed_s->fresh_seq()); i != nullptr; i = runtime::next_in_place(i))
          {
            v.push_back(i->first());
          }
          return make_box<obj::persistent_vector>(v.persistent());
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid sequence: {}", typed_s->to_string()) };
        }
      },
      s);
  }

  native_bool obj::persistent_vector::equal(object const &o) const
  {
    return detail::equal(o, data.begin(), data.end());
  }

  void obj::persistent_vector::to_string(fmt::memory_buffer &buff) const
  {
    behavior::detail::to_string(data.begin(), data.end(), "[", ']', buff);
  }

  native_persistent_string obj::persistent_vector::to_string() const
  {
    fmt::memory_buffer buff;
    behavior::detail::to_string(data.begin(), data.end(), "[", ']', buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_hash obj::persistent_vector::to_hash() const
  {
    if(hash != 0)
    {
      return hash;
    }

    return hash = hash::ordered(data.begin(), data.end());
  }

  obj::persistent_vector_sequence_ptr obj::persistent_vector::seq() const
  {
    if(data.empty())
    {
      return nullptr;
    }
    return make_box<obj::persistent_vector_sequence>(const_cast<obj::persistent_vector *>(this));
  }

  obj::persistent_vector_sequence_ptr obj::persistent_vector::fresh_seq() const
  {
    if(data.empty())
    {
      return nullptr;
    }
    return make_box<obj::persistent_vector_sequence>(const_cast<obj::persistent_vector *>(this));
  }

  size_t obj::persistent_vector::count() const
  {
    return data.size();
  }

  obj::persistent_vector_ptr obj::persistent_vector::conj(object_ptr head) const
  {
    auto vec(data.push_back(head));
    auto ret(make_box<obj::persistent_vector>(std::move(vec)));
    return ret;
  }

  obj::transient_vector_ptr obj::persistent_vector::to_transient() const
  {
    return make_box<obj::transient_vector>(data);
  }

  obj::persistent_vector_ptr obj::persistent_vector::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<obj::persistent_vector>(data));
    ret->meta = meta;
    return ret;
  }

  object_ptr obj::persistent_vector::get(object_ptr const key) const
  {
    if(key->type == object_type::integer)
    {
      auto const i(static_cast<size_t>(expect_object<obj::integer>(key)->data));
      if(data.size() <= i)
      {
        return obj::nil::nil_const();
      }
      return data[i];
    }
    else
    {
      throw std::runtime_error{ fmt::format("get on a vector must be an integer; found {}",
                                            runtime::detail::to_string(key)) };
    }
  }

  object_ptr obj::persistent_vector::get(object_ptr const key, object_ptr const fallback) const
  {
    if(key->type == object_type::integer)
    {
      auto const i(expect_object<obj::integer>(key)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return fallback;
      }
      return data[i];
    }
    else
    {
      throw std::runtime_error{ fmt::format("get on a vector must be an integer; found {}",
                                            runtime::detail::to_string(key)) };
    }
  }

  object_ptr obj::persistent_vector::get_entry(object_ptr const key) const
  {
    if(key->type == object_type::integer)
    {
      auto const i(expect_object<obj::integer>(key)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return obj::nil::nil_const();
      }
      /* TODO: Map entry type? */
      return make_box<obj::persistent_vector>(std::in_place, key, data[i]);
    }
    else
    {
      throw std::runtime_error{ fmt::format("get_entry on a vector must be an integer; found {}",
                                            runtime::detail::to_string(key)) };
    }
  }

  native_bool obj::persistent_vector::contains(object_ptr const key) const
  {
    if(key->type == object_type::integer)
    {
      auto const i(expect_object<obj::integer>(key)->data);
      return i >= 0 && static_cast<size_t>(i) < data.size();
    }
    else
    {
      return false;
    }
  }

  object_ptr obj::persistent_vector::peek() const
  {
    if(data.empty())
    {
      return obj::nil::nil_const();
    }

    return data[data.size() - 1];
  }

  obj::persistent_vector_ptr obj::persistent_vector::pop() const
  {
    if(data.empty())
    {
      throw std::runtime_error{ "cannot pop an empty vector" };
    }

    return make_box<obj::persistent_vector>(data.take(data.size() - 1));
  }

  object_ptr obj::persistent_vector::nth(object_ptr const index) const
  {
    if(index->type == object_type::integer)
    {
      auto const i(static_cast<size_t>(expect_object<obj::integer>(index)->data));
      if(data.size() <= i)
      {
        throw std::runtime_error{
          fmt::format("out of bounds index {}; vector has a size of {}", i, data.size())
        };
      }
      return data[i];
    }
    else
    {
      throw std::runtime_error{ fmt::format("nth on a vector must be an integer; found {}",
                                            runtime::detail::to_string(index)) };
    }
  }

  object_ptr obj::persistent_vector::nth(object_ptr const index, object_ptr const fallback) const
  {
    return get(index, fallback);
  }
}
