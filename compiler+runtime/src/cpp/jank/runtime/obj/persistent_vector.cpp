#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/transient_vector.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/seq_ext.hpp>
#include <jank/runtime/behavior/sequential.hpp>

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

  persistent_vector::persistent_vector(object_ptr const meta, value_type &&d)
    : data{ std::move(d) }
    , meta{ meta }
  {
  }

  persistent_vector::persistent_vector(option<object_ptr> const &meta, value_type &&d)
    : data{ std::move(d) }
    , meta{ meta }
  {
  }

  persistent_vector_ptr persistent_vector::create(object_ptr const s)
  {
    if(s == nullptr)
    {
      return make_box<persistent_vector>();
    }

    return visit_object(
      [](auto const typed_s) -> persistent_vector_ptr {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          runtime::detail::native_transient_vector v;
          for(auto i(typed_s->fresh_seq()); i != nullptr; i = runtime::next_in_place(i))
          {
            v.push_back(i->first());
          }
          return make_box<persistent_vector>(v.persistent());
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid sequence: {}", typed_s->to_string()) };
        }
      },
      s);
  }

  persistent_vector_ptr persistent_vector::empty()
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
    if(auto const v = dyn_cast<persistent_vector>(&o))
    {
      if(data.size() != v->data.size())
      {
        return false;
      }
      for(size_t i{}; i < data.size(); ++i)
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
      return visit_object(
        [&](auto const typed_o) -> native_bool {
          using T = typename decltype(typed_o)::value_type;

          if constexpr(behavior::sequential<T>)
          {
            size_t i{};
            auto e(typed_o->fresh_seq());
            for(; e != nullptr && i < data.size(); e = e->next_in_place(), ++i)
            {
              if(!runtime::equal(data[i], e->first()))
              {
                return false;
              }
            }
            return e == nullptr && i == data.size();
          }
          else
          {
            return false;
          }
        },
        &o);
    }
  }

  void persistent_vector::to_string(util::string_builder &buff) const
  {
    runtime::to_string(data.begin(), data.end(), "[", ']', buff);
  }

  native_persistent_string persistent_vector::to_string() const
  {
    util::string_builder buff;
    runtime::to_string(data.begin(), data.end(), "[", ']', buff);
    return buff.release();
  }

  native_persistent_string persistent_vector::to_code_string() const
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

  native_integer persistent_vector::compare(object const &o) const
  {
    return visit_type<persistent_vector>([this](auto const typed_o) { return compare(*typed_o); },
                                         &o);
  }

  native_integer persistent_vector::compare(persistent_vector const &v) const
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

    for(size_t i{}; i < size; ++i)
    {
      auto const res(runtime::compare(data[i], v.data[i]));
      if(res != 0)
      {
        return res;
      }
    }

    return 0;
  }

  persistent_vector_sequence_ptr persistent_vector::seq() const
  {
    return fresh_seq();
  }

  persistent_vector_sequence_ptr persistent_vector::fresh_seq() const
  {
    if(data.empty())
    {
      return nullptr;
    }
    return make_box<persistent_vector_sequence>(const_cast<persistent_vector *>(this));
  }

  size_t persistent_vector::count() const
  {
    return data.size();
  }

  persistent_vector_ptr persistent_vector::conj(object_ptr head) const
  {
    auto vec(data.push_back(head));
    auto ret(make_box<persistent_vector>(meta, std::move(vec)));
    return ret;
  }

  transient_vector_ptr persistent_vector::to_transient() const
  {
    return make_box<transient_vector>(data);
  }

  persistent_vector_ptr persistent_vector::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<persistent_vector>(data));
    ret->meta = meta;
    return ret;
  }

  object_ptr persistent_vector::get(object_ptr const key) const
  {
    return get(key, nil::nil_const());
  }

  object_ptr persistent_vector::get(object_ptr const key, object_ptr const fallback) const
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

  object_ptr persistent_vector::get_entry(object_ptr const key) const
  {
    if(key->type == object_type::integer)
    {
      auto const i(expect_object<integer>(key)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return nil::nil_const();
      }
      /* TODO: Map entry type? */
      return make_box<persistent_vector>(std::in_place, key, data[i]);
    }
    else
    {
      return nil::nil_const();
    }
  }

  native_bool persistent_vector::contains(object_ptr const key) const
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

  object_ptr persistent_vector::peek() const
  {
    if(data.empty())
    {
      return nil::nil_const();
    }

    return data[data.size() - 1];
  }

  persistent_vector_ptr persistent_vector::pop() const
  {
    if(data.empty())
    {
      throw std::runtime_error{ "cannot pop an empty vector" };
    }

    return make_box<persistent_vector>(meta, data.take(data.size() - 1));
  }

  object_ptr persistent_vector::nth(object_ptr const index) const
  {
    if(index->type == object_type::integer)
    {
      auto const i(expect_object<integer>(index)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
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
                                            runtime::to_string(index)) };
    }
  }

  object_ptr persistent_vector::nth(object_ptr const index, object_ptr const fallback) const
  {
    return get(index, fallback);
  }
}
