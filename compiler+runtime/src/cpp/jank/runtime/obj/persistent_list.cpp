#include <jank/runtime/util.hpp>
#include <jank/runtime/obj/persistent_list.hpp>

namespace jank::runtime
{
  obj::persistent_list::static_object(runtime::detail::native_persistent_list &&d)
    : data{ std::move(d) }
  {
  }

  obj::persistent_list::static_object(runtime::detail::native_persistent_list const &d)
    : data{ d }
  {
  }

  obj::persistent_list_ptr obj::persistent_list::create(object_ptr const s)
  {
    if(s == nullptr)
    {
      return make_box<obj::persistent_list>();
    }

    return visit_object(
      [](auto const typed_s) -> obj::persistent_list_ptr {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          native_vector<object_ptr> v;
          for(auto i(typed_s->fresh_seq()); i != nullptr; i = runtime::next_in_place(i))
          {
            v.emplace_back(i->first());
          }
          return make_box<obj::persistent_list>(
            runtime::detail::native_persistent_list{ v.rbegin(), v.rend() });
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid sequence: {}", typed_s->to_string()) };
        }
      },
      s);
  }

  obj::persistent_list_ptr obj::persistent_list::create(obj::persistent_list_ptr const s)
  {
    return s;
  }

  native_bool obj::persistent_list::equal(object const &o) const
  {
    return runtime::equal(o, data.begin(), data.end());
  }

  void obj::persistent_list::to_string(fmt::memory_buffer &buff) const
  {
    runtime::to_string(data.begin(), data.end(), "(", ')', buff);
  }

  native_persistent_string obj::persistent_list::to_string() const
  {
    fmt::memory_buffer buff;
    runtime::to_string(data.begin(), data.end(), "(", ')', buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  /* TODO: Cache this. */
  native_hash obj::persistent_list::to_hash() const
  {
    return hash::ordered(data.begin(), data.end());
  }

  obj::persistent_list_sequence_ptr obj::persistent_list::seq() const
  {
    if(data.empty())
    {
      return nullptr;
    }
    return make_box<obj::persistent_list_sequence>(this, data.begin(), data.end(), data.size());
  }

  obj::persistent_list_sequence_ptr obj::persistent_list::fresh_seq() const
  {
    if(data.empty())
    {
      return nullptr;
    }
    return make_box<obj::persistent_list_sequence>(this, data.begin(), data.end(), data.size());
  }

  size_t obj::persistent_list::count() const
  {
    return data.size();
  }

  obj::persistent_list_ptr obj::persistent_list::conj(object_ptr head) const
  {
    auto l(data.conj(head));
    auto ret(make_box<obj::persistent_list>(std::move(l)));
    return ret;
  }

  object_ptr obj::persistent_list::first() const
  {
    auto const first(data.first());
    if(first.is_none())
    {
      return obj::nil::nil_const();
    }
    return first.unwrap();
  }

  obj::persistent_list_sequence_ptr obj::persistent_list::next() const
  {
    if(data.size() < 2)
    {
      return nullptr;
    }
    return make_box<obj::persistent_list_sequence>(this,
                                                   ++data.begin(),
                                                   data.end(),
                                                   data.size() - 1);
  }

  obj::persistent_list_sequence_ptr obj::persistent_list::next_in_place() const
  {
    /* In-place updates don't make sense for lists, since any call to fresh_seq would return
     * a list sequence. So we know, principally, that a list itself cannot be considered fresh. */
    return next();
  }

  obj::persistent_list_ptr obj::persistent_list::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<obj::persistent_list>(data));
    ret->meta = meta;
    return ret;
  }

  object_ptr obj::persistent_list::peek() const
  {
    return data.first().unwrap_or(obj::nil::nil_const());
  }

  obj::persistent_list_ptr obj::persistent_list::pop() const
  {
    if(data.empty())
    {
      throw std::runtime_error{ "cannot pop an empty list" };
    }

    return make_box<obj::persistent_list>(data.rest());
  }
}
