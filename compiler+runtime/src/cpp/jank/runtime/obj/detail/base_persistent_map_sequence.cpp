#include <jank/runtime/obj/detail/base_persistent_map_sequence.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime::obj::detail
{
  template <typename PT, typename IT>
  base_persistent_map_sequence<PT, IT>::base_persistent_map_sequence(object_ref const c,
                                                                     IT const &b,
                                                                     IT const &e)
    : coll{ c }
    , begin{ b }
    , end{ e }
  {
    jank_debug_assert(begin != end);
  }

  template <typename PT, typename IT>
  bool base_persistent_map_sequence<PT, IT>::equal(object const &o) const
  {
    return runtime::sequence_equal(static_cast<PT const *>(this), &o);
  }

  template <typename PT, typename IT>
  void base_persistent_map_sequence<PT, IT>::to_string_impl(util::string_builder &buff,
                                                            bool const to_code) const
  {
    buff('(');
    for(auto i(begin); i != end; ++i)
    {
      buff('[');
      if(to_code)
      {
        runtime::to_code_string((*i).first, buff);
      }
      else
      {
        runtime::to_string((*i).first, buff);
      }
      buff(' ');
      if(to_code)
      {
        runtime::to_code_string((*i).second, buff);
      }
      else
      {
        runtime::to_string((*i).second, buff);
      }
      buff(']');
      auto n(i);
      if(++n != end)
      {
        buff(' ');
      }
    }
    buff(')');
  }

  template <typename PT, typename IT>
  void base_persistent_map_sequence<PT, IT>::to_string(util::string_builder &buff) const
  {
    return to_string_impl(buff, false);
  }

  template <typename PT, typename IT>
  jtl::immutable_string base_persistent_map_sequence<PT, IT>::to_string() const
  {
    util::string_builder buff;
    to_string_impl(buff, false);
    return buff.release();
  }

  template <typename PT, typename IT>
  jtl::immutable_string base_persistent_map_sequence<PT, IT>::to_code_string() const
  {
    util::string_builder buff;
    to_string_impl(buff, true);
    return buff.release();
  }

  template <typename PT, typename IT>
  uhash base_persistent_map_sequence<PT, IT>::to_hash() const
  {
    return hash::unordered(&static_cast<PT const *>(this)->base);
  }

  template <typename PT, typename IT>
  usize base_persistent_map_sequence<PT, IT>::count() const
  {
    return std::distance(begin, end);
  }

  template <typename PT, typename IT>
  oref<PT> base_persistent_map_sequence<PT, IT>::seq()
  {
    return static_cast<PT *>(this);
  }

  template <typename PT, typename IT>
  oref<PT> base_persistent_map_sequence<PT, IT>::fresh_seq() const
  {
    return make_box<PT>(coll, begin, end);
  }

  template <typename PT, typename IT>
  obj::persistent_vector_ref base_persistent_map_sequence<PT, IT>::first() const
  {
    auto const pair(*begin);
    return make_box<obj::persistent_vector>(
      runtime::detail::native_persistent_vector{ pair.first, pair.second });
  }

  template <typename PT, typename IT>
  oref<PT> base_persistent_map_sequence<PT, IT>::next() const
  {
    auto n(begin);
    ++n;

    if(n == end)
    {
      return {};
    }

    return make_box<PT>(coll, n, end);
  }

  template <typename PT, typename IT>
  oref<PT> base_persistent_map_sequence<PT, IT>::next_in_place()
  {
    ++begin;

    if(begin == end)
    {
      return {};
    }

    return static_cast<PT *>(this);
  }

  template <typename PT, typename IT>
  obj::cons_ref base_persistent_map_sequence<PT, IT>::conj(object_ref const head)
  {
    return make_box<obj::cons>(head, static_cast<PT *>(this));
  }

  template struct base_persistent_map_sequence<
    persistent_hash_map_sequence,
    runtime::detail::native_persistent_hash_map::const_iterator>;
  template struct base_persistent_map_sequence<persistent_array_map_sequence,
                                               runtime::detail::native_array_map::const_iterator>;
  template struct base_persistent_map_sequence<
    persistent_sorted_map_sequence,
    runtime::detail::native_persistent_sorted_map::const_iterator>;
}
