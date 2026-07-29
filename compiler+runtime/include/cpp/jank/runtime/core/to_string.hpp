#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime
{
  void to_string(char ch, jtl::string_builder &buff);
  void to_code_string(char ch, jtl::string_builder &buff);

  template <typename It>
  void to_string(It const &begin,
                 It const &end,
                 jtl::immutable_string_view const open,
                 char const close,
                 jtl::string_builder &buff)
  {
    for(auto const c : open)
    {
      buff(c);
    }
    for(auto i(begin); i != end; ++i)
    {
      buff((*i).to_string());
      auto n(i);
      if(++n != end)
      {
        buff(' ');
      }
    }
    buff(close);
  }

  template <typename T>
  void seq_to_string(oref<T> const &s, jtl::string_builder &buff)
  {
    if(s.is_nil())
    {
      buff("()");
      return;
    }

    buff('(');
    bool needs_space{};
    for(auto it{ s.fresh_seq() }; it.is_some(); it = it.next_in_place())
    {
      if(needs_space)
      {
        buff(' ');
      }
      buff(it.first().to_code_string());
      needs_space = true;
    }
    buff(')');
  }

  template <typename T>
  jtl::immutable_string seq_to_string(oref<T> const &s)
  {
    jtl::string_builder buff;
    runtime::seq_to_string(s, buff);
    return buff.release();
  }

  template <typename It>
  void to_code_string(It const &begin,
                      It const &end,
                      jtl::immutable_string_view const open,
                      char const close,
                      jtl::string_builder &buff)
  {
    for(auto const c : open)
    {
      buff(c);
    }
    for(auto i(begin); i != end; ++i)
    {
      buff((*i).to_code_string());
      auto n(i);
      if(++n != end)
      {
        buff(' ');
      }
    }
    buff(close);
  }

  template <typename T>
  void seq_to_code_string(oref<T> const &s, jtl::string_builder &buff)
  {
    if(s.is_nil())
    {
      buff("()");
      return;
    }

    buff('(');
    bool needs_space{};
    for(auto it{ s.fresh_seq() }; it.is_some(); it = it.next_in_place())
    {
      if(needs_space)
      {
        buff(' ');
      }
      buff(it.first().to_code_string());
      needs_space = true;
    }
    buff(')');
  }

  template <typename T>
  jtl::immutable_string seq_to_code_string(oref<T> const &s)
  {
    jtl::string_builder buff;
    runtime::seq_to_code_string(s, buff);
    return buff.release();
  }
}
