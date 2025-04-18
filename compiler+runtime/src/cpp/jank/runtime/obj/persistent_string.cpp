#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/persistent_string_sequence.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/util/escape.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  persistent_string::persistent_string(jtl::immutable_string const &d)
    : data{ d }
  {
  }

  persistent_string::persistent_string(jtl::immutable_string &&d)
    : data{ std::move(d) }
  {
  }

  native_bool persistent_string::equal(object const &o) const
  {
    if(o.type != object_type::persistent_string)
    {
      return false;
    }

    auto const s(expect_object<persistent_string>(&o));
    return data == s->data;
  }

  jtl::immutable_string const &persistent_string::to_string() const
  {
    return data;
  }

  void persistent_string::to_string(util::string_builder &buff) const
  {
    buff(data);
  }

  jtl::immutable_string persistent_string::to_code_string() const
  {
    util::string_builder sb;
    return sb('"')(util::escape(data))('"').release();
  }

  native_hash persistent_string::to_hash() const
  {
    return data.to_hash();
  }

  i64 persistent_string::compare(object const &o) const
  {
    return compare(*try_object<persistent_string>(&o));
  }

  i64 persistent_string::compare(persistent_string const &s) const
  {
    return data.compare(s.data);
  }

  object_ref persistent_string::get(object_ref const key) const
  {
    return get(key, jank_nil);
  }

  object_ref persistent_string::get(object_ref const key, object_ref const fallback) const
  {
    if(key->type == object_type::integer)
    {
      auto const i(expect_object<integer>(key)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return fallback;
      }
      return make_box<character>(data[i]);
    }
    else
    {
      return fallback;
    }
  }

  native_bool persistent_string::contains(object_ref const key) const
  {
    if(key->type == object_type::integer)
    {
      auto const i(expect_object<integer>(key)->data);
      return 0 <= i && static_cast<size_t>(i) < data.size();
    }
    return false;
  }

  object_ref persistent_string::get_entry(object_ref const) const
  {
    throw std::runtime_error{ util::format("get_entry not supported on string") };
  }

  object_ref persistent_string::nth(object_ref const index) const
  {
    if(index->type == object_type::integer)
    {
      auto const i(expect_object<integer>(index)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        throw std::runtime_error{
          util::format("out of bounds index {}; string has a size of {}", i, data.size())
        };
      }
      return make_box<character>(data[i]);
    }
    else
    {
      throw std::runtime_error{ util::format("nth on a string must be an integer; found {}",
                                             runtime::to_string(index)) };
    }
  }

  object_ref persistent_string::nth(object_ref const index, object_ref const fallback) const
  {
    return get(index, fallback);
  }

  jtl::string_result<persistent_string_ref> persistent_string::substring(i64 start) const
  {
    return substring(start, static_cast<i64>(data.size()));
  }

  jtl::string_result<persistent_string_ref>
  persistent_string::substring(i64 const start, i64 const end) const
  {
    if(start < 0)
    {
      return err(util::format("start index {} is less than 0", start));
    }
    if(end < start)
    {
      return err(util::format("end index {} is less than start {}", end, start));
    }
    else if(static_cast<size_t>(end) > data.size())
    {
      return err(util::format("end index {} out of bounds for length {}", end, data.size()));
    }

    return ok(make_box(data.substr(start, end - start)));
  }

  i64 persistent_string::first_index_of(object_ref const m) const
  {
    auto const s(runtime::to_string(m));
    auto const found(data.find(s));
    if(found == jtl::immutable_string::npos)
    {
      return -1;
    }
    return static_cast<i64>(found);
  }

  i64 persistent_string::last_index_of(object_ref const m) const
  {
    auto const s(runtime::to_string(m));
    auto const found(data.rfind(s));
    if(found == jtl::immutable_string::npos)
    {
      return -1;
    }
    return static_cast<i64>(found);
  }

  usize persistent_string::count() const
  {
    return data.size();
  }

  persistent_string_sequence_ref persistent_string::seq() const
  {
    return fresh_seq();
  }

  persistent_string_sequence_ref persistent_string::fresh_seq() const
  {
    if(data.empty())
    {
      return {};
    }
    return make_box<persistent_string_sequence>(const_cast<persistent_string *>(this));
  }
}
