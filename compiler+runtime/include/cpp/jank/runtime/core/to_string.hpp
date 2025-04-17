#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/sequence_range.hpp>

namespace jank::runtime
{
  jtl::immutable_string to_string(object_ref const o);
  void to_string(char ch, util::string_builder &buff);
  void to_string(object_ref o, util::string_builder &buff);

  jtl::immutable_string to_code_string(object_ref const o);
  void to_code_string(char ch, util::string_builder &buff);
  void to_code_string(object_ref o, util::string_builder &buff);

  template <typename It>
  void to_string(It const &begin,
                 It const &end,
                 native_persistent_string_view const open,
                 char const close,
                 util::string_builder &buff)
  {
    for(auto const c : open)
    {
      buff(c);
    }
    for(auto i(begin); i != end; ++i)
    {
      runtime::to_string(*i, buff);
      auto n(i);
      if(++n != end)
      {
        buff(' ');
      }
    }
    buff(close);
  }

  template <typename T>
  requires behavior::sequenceable<T>
  void to_string(oref<T> const s, util::string_builder &buff)
  {
    if(s.is_nil())
    {
      buff("()");
      return;
    }

    buff('(');
    native_bool needs_space{};
    for(auto const e : make_sequence_range(s))
    {
      if(needs_space)
      {
        buff(' ');
      }
      runtime::to_string(e.erase(), buff);
      needs_space = true;
    }
    buff(')');
  }

  template <typename T>
  requires behavior::sequenceable<T>
  jtl::immutable_string to_string(oref<T> const s)
  {
    util::string_builder buff;
    runtime::to_string(s, buff);
    return buff.release();
  }

  template <typename It>
  void to_code_string(It const &begin,
                      It const &end,
                      native_persistent_string_view const open,
                      char const close,
                      util::string_builder &buff)
  {
    for(auto const c : open)
    {
      buff(c);
    }
    for(auto i(begin); i != end; ++i)
    {
      runtime::to_code_string(*i, buff);
      auto n(i);
      if(++n != end)
      {
        buff(' ');
      }
    }
    buff(close);
  }

  template <typename T>
  requires behavior::sequenceable<T>
  void to_code_string(oref<T> const s, util::string_builder &buff)
  {
    if(s.is_nil())
    {
      buff("()");
      return;
    }

    buff('(');
    native_bool needs_space{};
    for(auto const e : make_sequence_range(s))
    {
      if(needs_space)
      {
        buff(' ');
      }
      runtime::to_code_string(e.erase(), buff);
      needs_space = true;
    }
    buff(')');
  }

  template <typename T>
  requires behavior::sequenceable<T>
  jtl::immutable_string to_code_string(oref<T> const s)
  {
    util::string_builder buff;
    runtime::to_code_string(s, buff);
    return buff.release();
  }
}
