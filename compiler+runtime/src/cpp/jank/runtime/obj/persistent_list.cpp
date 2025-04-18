#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/seq_ext.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  persistent_list::persistent_list(runtime::detail::native_persistent_list const &d)
    : data{ d }
  {
  }

  persistent_list::persistent_list(object_ref const meta,
                                   runtime::detail::native_persistent_list const &d)
    : data{ d }
    , meta{ meta }
  {
  }

  persistent_list_ref persistent_list::create(object_ref const meta, object_ref const s)
  {
    auto const ret{ create(s) };
    auto const m{ behavior::detail::validate_meta(meta) };
    ret->meta = m;
    return ret;
  }

  persistent_list_ref persistent_list::create(object_ref const s)
  {
    if(s.is_nil())
    {
      return make_box<persistent_list>();
    }

    return visit_object(
      [](auto const typed_s) -> persistent_list_ref {
        using T = typename decltype(typed_s)::value_type;

        if constexpr(behavior::sequenceable<T> || std::same_as<T, nil>)
        {
          native_vector<object_ref> v;
          for(auto const e : make_sequence_range(typed_s))
          {
            v.emplace_back(e);
          }
          return make_box<persistent_list>(
            runtime::detail::native_persistent_list{ v.rbegin(), v.rend() });
        }
        else
        {
          throw std::runtime_error{ util::format("invalid sequence: {}", typed_s->to_string()) };
        }
      },
      s);
  }

  persistent_list_ref persistent_list::create(persistent_list_ref const s)
  {
    return s;
  }

  persistent_list_ref persistent_list::create(nil_ref const)
  {
    return empty();
  }

  native_bool persistent_list::equal(object const &o) const
  {
    return runtime::equal(o, data.begin(), data.end());
  }

  void persistent_list::to_string(util::string_builder &buff) const
  {
    runtime::to_string(data.begin(), data.end(), "(", ')', buff);
  }

  jtl::immutable_string persistent_list::to_string() const
  {
    util::string_builder buff;
    runtime::to_string(data.begin(), data.end(), "(", ')', buff);
    return buff.release();
  }

  jtl::immutable_string persistent_list::to_code_string() const
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

  persistent_list_sequence_ref persistent_list::seq() const
  {
    return fresh_seq();
  }

  persistent_list_sequence_ref persistent_list::fresh_seq() const
  {
    if(data.empty())
    {
      return {};
    }
    return make_box<persistent_list_sequence>(this, data.begin(), data.end(), data.size());
  }

  usize persistent_list::count() const
  {
    return data.size();
  }

  persistent_list_ref persistent_list::conj(object_ref head) const
  {
    auto l(data.conj(head));
    auto ret(make_box<persistent_list>(std::move(l)));
    return ret;
  }

  object_ref persistent_list::first() const
  {
    auto const first(data.first());
    if(first.is_none())
    {
      return jank_nil;
    }
    return first.unwrap();
  }

  persistent_list_sequence_ref persistent_list::next() const
  {
    if(data.size() < 2)
    {
      return {};
    }
    return make_box<persistent_list_sequence>(this, ++data.begin(), data.end(), data.size() - 1);
  }

  persistent_list_sequence_ref persistent_list::next_in_place() const
  {
    /* In-place updates don't make sense for lists, since any call to fresh_seq would return
     * a list sequence. So we know, principally, that a list itself cannot be considered fresh. */
    return next();
  }

  persistent_list_ref persistent_list::with_meta(object_ref const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<persistent_list>(data));
    ret->meta = meta;
    return ret;
  }

  object_ref persistent_list::peek() const
  {
    return data.first().unwrap_or(jank_nil);
  }

  persistent_list_ref persistent_list::pop() const
  {
    if(data.empty())
    {
      throw std::runtime_error{ "cannot pop an empty list" };
    }

    return make_box<persistent_list>(data.rest());
  }
}
