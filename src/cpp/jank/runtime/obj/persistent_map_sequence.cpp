#include <jank/runtime/obj/persistent_map_sequence.hpp>

namespace jank::runtime
{
  obj::persistent_map_sequence::static_object
  (
    object_ptr c,
    obj::persistent_map_sequence::iterator_type const &b,
    obj::persistent_map_sequence::iterator_type const &e
  )
    : coll{ c }, begin{ b }, end{ e }
  { }

  native_bool obj::persistent_map_sequence::equal(object const &o) const
  {
    return visit_object
    (
      [this](auto const typed_o)
      {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(!behavior::seqable<T>)
        { return false; }
        else
        {
          auto seq(typed_o->fresh_seq());
          for(auto it(fresh_seq()); it != nullptr; seq = seq->next_in_place(), seq = seq->next_in_place())
          {
            if(seq == nullptr || !runtime::detail::equal(it, seq->first()))
            { return false; }
          }
          return true;
        }
      },
      &o
    );
  }

  void obj::persistent_map_sequence::to_string_impl(fmt::memory_buffer &buff) const
  {
    auto inserter(std::back_inserter(buff));
    fmt::format_to(inserter, "(");
    for(auto i(begin); i != end; ++i)
    {
      fmt::format_to(inserter, "[");
      runtime::detail::to_string((*i).first, buff);
      fmt::format_to(inserter, " ");
      runtime::detail::to_string((*i).second, buff);
      fmt::format_to(inserter, "]");
      auto n(i);
      if(++n != end)
      { fmt::format_to(inserter, " "); }
    }
    fmt::format_to(inserter, ")");
  }

  void obj::persistent_map_sequence::to_string(fmt::memory_buffer &buff) const
  { return to_string_impl(buff); }

  native_string obj::persistent_map_sequence::to_string() const
  {
    fmt::memory_buffer buff;
    to_string_impl(buff);
    return native_string{ buff.data(), buff.size() };
  }

  native_integer obj::persistent_map_sequence::to_hash() const
  /* TODO: Hash from contents. */
  { return reinterpret_cast<native_integer>(this); }

  size_t obj::persistent_map_sequence::count() const
  { return std::distance(begin, end); }

  obj::persistent_map_sequence_ptr obj::persistent_map_sequence::seq()
  { return this; }

  obj::persistent_map_sequence_ptr obj::persistent_map_sequence::fresh_seq() const
  { return jank::make_box<obj::persistent_map_sequence>(coll, begin, end); }

  object_ptr obj::persistent_map_sequence::first() const
  {
    auto const pair(*begin);
    return jank::make_box<obj::vector>(runtime::detail::peristent_vector{ pair.first, pair.second });
  }

  obj::persistent_map_sequence_ptr obj::persistent_map_sequence::next() const
  {
    auto n(begin);
    ++n;

    if(n == end)
    { return nullptr; }

    return jank::make_box<obj::persistent_map_sequence>(coll, n, end);
  }

  obj::persistent_map_sequence_ptr obj::persistent_map_sequence::next_in_place()
  {
    ++begin;

    if(begin == end)
    { return nullptr; }

    return this;
  }
  object_ptr obj::persistent_map_sequence::next_in_place_first()
  {
    ++begin;

    if(begin == end)
    { return nullptr; }

    auto const pair(*begin);
    return jank::make_box<obj::vector>(runtime::detail::peristent_vector{ pair.first, pair.second });
  }

  obj::cons_ptr obj::persistent_map_sequence::cons(object_ptr const head)
  { return make_box<obj::cons>(head, this); }
}
