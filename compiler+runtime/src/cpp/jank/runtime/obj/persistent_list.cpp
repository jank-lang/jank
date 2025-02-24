#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/seq_ext.hpp>

namespace jank::runtime::obj
{
  persistent_list::persistent_list(runtime::detail::native_persistent_list const &d)
    : data{ d }
  {
  }

  persistent_list::persistent_list(object_ptr const meta,
                                   runtime::detail::native_persistent_list const &d)
    : data{ d }
    , meta{ meta }
  {
  }

  persistent_list_ptr persistent_list::create(object_ptr const s)
  {
    if(s == nullptr)
    {
      return make_box<persistent_list>();
    }

    return visit_object(
      [](auto const typed_s) -> persistent_list_ptr {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(behavior::sequenceable<T> || std::same_as<T, nil>)
        {
          native_vector<object_ptr> v;
          for(auto i(typed_s->fresh_seq()); i != nullptr; i = runtime::next_in_place(i))
          {
            v.emplace_back(i->first());
          }
          return make_box<persistent_list>(
            runtime::detail::native_persistent_list{ v.rbegin(), v.rend() });
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid sequence: {}", typed_s->to_string()) };
        }
      },
      s);
  }

  persistent_list_ptr persistent_list::create(persistent_list_ptr const s)
  {
    return s;
  }

  native_bool persistent_list::equal(object const &o) const
  {
    return runtime::equal(o, data.begin(), data.end());
  }

  void persistent_list::to_string(util::string_builder &buff) const
  {
    runtime::to_string(data.begin(), data.end(), "(", ')', buff);
  }

  native_persistent_string persistent_list::to_string() const
  {
    util::string_builder buff;
    runtime::to_string(data.begin(), data.end(), "(", ')', buff);
    return buff.release();
  }

  native_persistent_string persistent_list::to_code_string() const
  {
    util::string_builder buff;
    runtime::to_code_string(data.begin(), data.end(), "(", ')', buff);
    return buff.release();
  }

  /* TODO: Cache this. */
  native_hash persistent_list::to_hash() const
  {
    return hash::ordered(data.begin(), data.end());
  }

  persistent_list_sequence_ptr persistent_list::seq() const
  {
    return fresh_seq();
  }

  persistent_list_sequence_ptr persistent_list::fresh_seq() const
  {
    if(data.empty())
    {
      return nullptr;
    }
    return make_box<persistent_list_sequence>(this, data.begin(), data.end(), data.size());
  }

  size_t persistent_list::count() const
  {
    return data.size();
  }

  persistent_list_ptr persistent_list::conj(object_ptr head) const
  {
    auto l(data.conj(head));
    auto ret(make_box<persistent_list>(std::move(l)));
    return ret;
  }

  object_ptr persistent_list::first() const
  {
    auto const first(data.first());
    if(first.is_none())
    {
      return nil::nil_const();
    }
    return first.unwrap();
  }

  persistent_list_sequence_ptr persistent_list::next() const
  {
    if(data.size() < 2)
    {
      return nullptr;
    }
    return make_box<persistent_list_sequence>(this, ++data.begin(), data.end(), data.size() - 1);
  }

  persistent_list_sequence_ptr persistent_list::next_in_place() const
  {
    /* In-place updates don't make sense for lists, since any call to fresh_seq would return
     * a list sequence. So we know, principally, that a list itself cannot be considered fresh. */
    return next();
  }

  persistent_list_ptr persistent_list::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<persistent_list>(data));
    ret->meta = meta;
    return ret;
  }

  object_ptr persistent_list::peek() const
  {
    return data.first().unwrap_or(nil::nil_const());
  }

  persistent_list_ptr persistent_list::pop() const
  {
    if(data.empty())
    {
      throw std::runtime_error{ "cannot pop an empty list" };
    }

    return make_box<persistent_list>(data.rest());
  }
}
