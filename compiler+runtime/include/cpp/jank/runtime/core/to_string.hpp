#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime
{
  native_persistent_string to_string(object const *o);
  void to_string(char ch, fmt::memory_buffer &buff);
  void to_string(object_ptr o, fmt::memory_buffer &buff);

  native_persistent_string to_code_string(object const *o);
  void to_code_string(char ch, fmt::memory_buffer &buff);
  void to_code_string(object_ptr o, fmt::memory_buffer &buff);

  template <typename It>
  void to_string(It const &begin,
                 It const &end,
                 native_persistent_string_view const open,
                 char const close,
                 fmt::memory_buffer &buff)
  {
    auto inserter(std::back_inserter(buff));
    for(auto const c : open)
    {
      inserter = c;
    }
    for(auto i(begin); i != end; ++i)
    {
      runtime::to_string(*i, buff);
      auto n(i);
      if(++n != end)
      {
        inserter = ' ';
      }
    }
    inserter = close;
  }

  template <typename T>
  requires behavior::sequenceable<T>
  void to_string(native_box<T> const s, fmt::memory_buffer &buff)
  {
    auto inserter(std::back_inserter(buff));
    if(!s)
    {
      fmt::format_to(inserter, "()");
      return;
    }

    fmt::format_to(inserter, "(");
    native_bool needs_space{};
    for(auto i(s->fresh_seq()); i != nullptr; i = i->next_in_place())
    {
      if(needs_space)
      {
        fmt::format_to(inserter, " ");
      }
      runtime::to_string(i->first(), buff);
      needs_space = true;
    }
    fmt::format_to(inserter, ")");
  }

  template <typename T>
  requires behavior::sequenceable<T>
  native_persistent_string to_string(native_box<T> const s)
  {
    fmt::memory_buffer buff;
    runtime::to_string(s, buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  template <typename It>
  void to_code_string(It const &begin,
                      It const &end,
                      native_persistent_string_view const open,
                      char const close,
                      fmt::memory_buffer &buff)
  {
    auto inserter(std::back_inserter(buff));
    for(auto const c : open)
    {
      inserter = c;
    }
    for(auto i(begin); i != end; ++i)
    {
      runtime::to_code_string(*i, buff);
      auto n(i);
      if(++n != end)
      {
        inserter = ' ';
      }
    }
    inserter = close;
  }

  template <typename T>
  requires behavior::sequenceable<T>
  void to_code_string(native_box<T> const s, fmt::memory_buffer &buff)
  {
    auto inserter(std::back_inserter(buff));
    if(!s)
    {
      fmt::format_to(inserter, "()");
      return;
    }

    fmt::format_to(inserter, "(");
    native_bool needs_space{};
    for(auto i(s->fresh_seq()); i != nullptr; i = i->next_in_place())
    {
      if(needs_space)
      {
        fmt::format_to(inserter, " ");
      }
      runtime::to_code_string(i->first(), buff);
      needs_space = true;
    }
    fmt::format_to(inserter, ")");
  }

  template <typename T>
  requires behavior::sequenceable<T>
  native_persistent_string to_code_string(native_box<T> const s)
  {
    fmt::memory_buffer buff;
    runtime::to_code_string(s, buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }
}
