#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime
{
  object_ref first(object_ref s);
  object_ref next(object_ref s);

  jtl::immutable_string to_string(object_ref const o);
  void to_string(char ch, util::string_builder &buff);
  void to_string(object_ref o, util::string_builder &buff);

  jtl::immutable_string to_code_string(object_ref const o);
  void to_code_string(char ch, util::string_builder &buff);
  void to_code_string(object_ref o, util::string_builder &buff);

  template <typename T>
  requires(behavior::object_like<T> && !behavior::sequenceable<T>)
  void to_string(oref<T> const s, util::string_builder &buff)
  {
    s->to_string(buff);
  }

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
    bool needs_space{};
    if constexpr(behavior::sequenceable_in_place<T>)
    {
      for(auto it{ s->fresh_seq() }; it.is_some(); it = it->next_in_place())
      {
        if(needs_space)
        {
          buff(' ');
        }
        runtime::to_string(it->first(), buff);
        needs_space = true;
      }
    }
    else
    {
      for(object_ref it{ s->seq() }; it.is_some(); it = runtime::next(it))
      {
        if(needs_space)
        {
          buff(' ');
        }
        runtime::to_string(runtime::first(it), buff);
        needs_space = true;
      }
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

  template <typename T>
  requires(behavior::object_like<T> && !behavior::sequenceable<T>)
  void to_code_string(oref<T> const s, util::string_builder &buff)
  {
    buff(s->to_code_string());
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
    bool needs_space{};
    if constexpr(behavior::sequenceable_in_place<T>)
    {
      for(auto it{ s->fresh_seq() }; it.is_some(); it = it->next_in_place())
      {
        if(needs_space)
        {
          buff(' ');
        }
        runtime::to_code_string(it->first(), buff);
        needs_space = true;
      }
    }
    else
    {
      for(object_ref it{ s->seq() }; it.is_some(); it = runtime::next(it))
      {
        if(needs_space)
        {
          buff(' ');
        }
        runtime::to_code_string(runtime::first(it), buff);
        needs_space = true;
      }
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
