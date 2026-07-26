#include <jtl/utf8.hpp>

#include <jank/runtime/obj/persistent_string_sequence.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/character.hpp>
#include <jank/runtime/core/seq_ext.hpp>
#include <jank/runtime/core/make_box.hpp>

namespace jank::runtime::obj
{
  persistent_string_sequence::persistent_string_sequence()
    : object{ obj_type, obj_behaviors }
  {
  }

  persistent_string_sequence::persistent_string_sequence(persistent_string_ref const s)
    : object{ obj_type, obj_behaviors }
    , str{ s }
  {
    jank_debug_assert(!s->data.empty());
  }

  persistent_string_sequence::persistent_string_sequence(persistent_string_ref const s,
                                                         usize const i)
    : object{ obj_type, obj_behaviors }
    , str{ s }
    , index{ i }
  {
    jank_debug_assert(!s->data.empty() && i < s->data.size());
  }

  /* behavior::object_like */
  bool persistent_string_sequence::equal(object const &o) const
  {
    return runtime::equal(o, str->data.begin() + index, str->data.end());
  }

  void persistent_string_sequence::to_string(jtl::string_builder &buff) const
  {
    auto const range(jtl::utf8_range(str->data.substr(index)));
    auto const begin(range.begin());
    auto const end(range.end());

    buff('(');
    for(auto i(begin); i != end; ++i)
    {
      buff(character{ *i }.to_code_string());
      auto n(i);
      if(++n != end)
      {
        buff(' ');
      }
    }
    buff(')');
  }

  jtl::immutable_string persistent_string_sequence::to_string() const
  {
    jtl::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  jtl::immutable_string persistent_string_sequence::to_code_string() const
  {
    jtl::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  uhash persistent_string_sequence::to_hash() const
  {
    return hash::ordered(str->data.begin() + index, str->data.end());
  }

  /* behavior::countable */
  usize persistent_string_sequence::count() const
  {
    usize count{};
    auto const size(str->data.size());
    for(auto i(index); i < size; i += jtl::next_char_size(str->data, i))
    {
      ++count;
    }
    return count;
  }

  /* behavior::seqable */
  persistent_string_sequence_ref persistent_string_sequence::seq()
  {
    return runtime::detail::untagged(this);
  }

  persistent_string_sequence_ref persistent_string_sequence::fresh_seq() const
  {
    return make_box<persistent_string_sequence>(str, index);
  }

  /* behavior::sequenceable */
  object_ref persistent_string_sequence::first() const
  {
    auto const size(jtl::next_char_size(str->data, index));
    return make_box<character>(str->data.substr(index, size));
  }

  persistent_string_sequence_ref persistent_string_sequence::next() const
  {
    auto n(index + jtl::next_char_size(str->data, index));

    if(n == str->data.size())
    {
      return {};
    }

    return make_box<persistent_string_sequence>(str, n);
  }

  persistent_string_sequence_ref persistent_string_sequence::next_in_place()
  {
    index += jtl::next_char_size(str->data, index);

    if(index == str->data.size())
    {
      return {};
    }

    return runtime::detail::untagged(this);
  }

  cons_ref persistent_string_sequence::conj(object_ref const head)
  {
    return make_box<cons>(head, runtime::detail::untagged(this));
  }
}
