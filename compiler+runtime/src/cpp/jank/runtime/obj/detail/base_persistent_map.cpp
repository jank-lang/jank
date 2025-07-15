#include <jank/runtime/obj/detail/base_persistent_map.hpp>
#include <jank/runtime/behavior/associatively_readable.hpp>
#include <jank/runtime/behavior/map_like.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj::detail
{
  template <typename PT, typename ST, typename V>
  base_persistent_map<PT, ST, V>::base_persistent_map(jtl::option<object_ref> const &meta)
    : meta{ meta }
  {
  }

  template <typename PT, typename ST, typename V>
  bool base_persistent_map<PT, ST, V>::equal(object const &o) const
  {
    if(&o == &base)
    {
      return true;
    }

    return visit_map_like(
      [&](auto const typed_o) -> bool {
        if(typed_o->count() != count())
        {
          return false;
        }

        for(auto const &entry : static_cast<PT const *>(this)->data)
        {
          auto const found(typed_o->contains(entry.first));

          if(!found || !runtime::equal(entry.second, typed_o->get(entry.first)))
          {
            return false;
          }
        }

        return true;
      },
      []() { return false; },
      &o);
  }

  template <typename PT, typename ST, typename V>
  void base_persistent_map<PT, ST, V>::to_string_impl(typename V::const_iterator const &begin,
                                                      typename V::const_iterator const &end,
                                                      util::string_builder &buff,
                                                      bool const to_code)
  {
    auto inserter(std::back_inserter(buff));
    inserter = '{';
    for(auto i(begin); i != end; ++i)
    {
      auto const pair(*i);
      if(to_code)
      {
        runtime::to_code_string(pair.first, buff);
      }
      else
      {
        runtime::to_string(pair.first, buff);
      }
      inserter = ' ';

      if(to_code)
      {
        runtime::to_code_string(pair.second, buff);
      }
      else
      {
        runtime::to_string(pair.second, buff);
      }
      auto n(i);
      if(++n != end)
      {
        inserter = ',';
        inserter = ' ';
      }
    }
    inserter = '}';
  }

  template <typename PT, typename ST, typename V>
  void base_persistent_map<PT, ST, V>::to_string(util::string_builder &buff) const
  {
    to_string_impl(static_cast<PT const *>(this)->data.begin(),
                   static_cast<PT const *>(this)->data.end(),
                   buff,
                   false);
  }

  template <typename PT, typename ST, typename V>
  jtl::immutable_string base_persistent_map<PT, ST, V>::to_string() const
  {
    util::string_builder buff;
    to_string_impl(static_cast<PT const *>(this)->data.begin(),
                   static_cast<PT const *>(this)->data.end(),
                   buff,
                   false);
    return buff.release();
  }

  template <typename PT, typename ST, typename V>
  jtl::immutable_string base_persistent_map<PT, ST, V>::to_code_string() const
  {
    util::string_builder buff;
    to_string_impl(static_cast<PT const *>(this)->data.begin(),
                   static_cast<PT const *>(this)->data.end(),
                   buff,
                   true);
    return buff.release();
  }

  template <typename PT, typename ST, typename V>
  uhash base_persistent_map<PT, ST, V>::to_hash() const
  {
    if(hash)
    {
      return hash;
    }

    return hash = hash::unordered(static_cast<PT const *>(this)->data.begin(),
                                  static_cast<PT const *>(this)->data.end());
  }

  template <typename PT, typename ST, typename V>
  oref<ST> base_persistent_map<PT, ST, V>::seq() const
  {
    if(static_cast<PT const *>(this)->data.empty())
    {
      return {};
    }
    return make_box<ST>(static_cast<PT const *>(this),
                        static_cast<PT const *>(this)->data.begin(),
                        static_cast<PT const *>(this)->data.end());
  }

  template <typename PT, typename ST, typename V>
  oref<ST> base_persistent_map<PT, ST, V>::fresh_seq() const
  {
    if(static_cast<PT const *>(this)->data.empty())
    {
      return {};
    }
    return make_box<ST>(static_cast<PT const *>(this),
                        static_cast<PT const *>(this)->data.begin(),
                        static_cast<PT const *>(this)->data.end());
  }

  template <typename PT, typename ST, typename V>
  usize base_persistent_map<PT, ST, V>::count() const
  {
    return static_cast<PT const *>(this)->data.size();
  }

  template <typename PT, typename ST, typename V>
  object_ref base_persistent_map<PT, ST, V>::conj(object_ref const head) const
  {
    auto const ret(static_cast<PT const *>(this));
    if(head.is_nil())
    {
      return ret;
    }

    if(is_map(head))
    {
      return merge(ret, head);
    }

    if(head->type != object_type::persistent_vector)
    {
      throw std::runtime_error{ util::format("invalid map entry: {}",
                                             runtime::to_code_string(head)) };
    }

    auto const vec(expect_object<obj::persistent_vector>(head));
    if(vec->count() != 2)
    {
      throw std::runtime_error{ util::format("invalid map entry: {}",
                                             runtime::to_code_string(head)) };
    }

    return ret->assoc(vec->data[0], vec->data[1]);
  }

  template <typename PT, typename ST, typename V>
  oref<PT> base_persistent_map<PT, ST, V>::with_meta(object_ref const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<PT>(static_cast<PT const *>(this)->data));
    ret->meta = meta;
    return ret;
  }

  template struct base_persistent_map<persistent_array_map,
                                      persistent_array_map_sequence,
                                      runtime::detail::native_array_map>;
  template struct base_persistent_map<persistent_hash_map,
                                      persistent_hash_map_sequence,
                                      runtime::detail::native_persistent_hash_map>;
  template struct base_persistent_map<persistent_sorted_map,
                                      persistent_sorted_map_sequence,
                                      runtime::detail::native_persistent_sorted_map>;
}
