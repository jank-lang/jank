#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>

namespace jank::runtime
{
  void to_string(object_ptr o, fmt::memory_buffer &buff);
  void to_code_string(object_ptr o, fmt::memory_buffer &buff);
}

namespace jank::runtime::obj::detail
{
  template <object_type OT, typename It>
  struct base_persistent_map_sequence : gc
  {
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    using parent_type = static_object<OT>;
    using iterator_type = It;

    base_persistent_map_sequence() = default;
    base_persistent_map_sequence(base_persistent_map_sequence &&) = default;
    base_persistent_map_sequence(base_persistent_map_sequence const &) = default;

    base_persistent_map_sequence(object_ptr c, iterator_type const &b, iterator_type const &e)
      : coll{ c }
      , begin{ b }
      , end{ e }
    {
      assert(begin != end);
    }

    /* behavior::object_like */
    native_bool equal(object const &o) const
    {
      return visit_seqable(
        [this](auto const typed_o) {
          auto seq(typed_o->fresh_seq());
          for(auto it(fresh_seq()); it != nullptr;
              it = it->next_in_place(), seq = seq->next_in_place())
          {
            if(seq == nullptr || !runtime::equal(it, seq->first()))
            {
              return false;
            }
          }
          return true;
        },
        []() { return false; },
        &o);
    }

    void to_string_impl(fmt::memory_buffer &buff, native_bool const to_code) const
    {
      auto inserter(std::back_inserter(buff));
      fmt::format_to(inserter, "(");
      for(auto i(begin); i != end; ++i)
      {
        fmt::format_to(inserter, "[");
        if(to_code)
        {
          runtime::to_code_string((*i).first, buff);
        }
        else
        {
          runtime::to_string((*i).first, buff);
        }
        fmt::format_to(inserter, " ");
        if(to_code)
        {
          runtime::to_code_string((*i).second, buff);
        }
        else
        {
          runtime::to_string((*i).second, buff);
        }
        fmt::format_to(inserter, "]");
        auto n(i);
        if(++n != end)
        {
          fmt::format_to(inserter, " ");
        }
      }
      fmt::format_to(inserter, ")");
    }

    void to_string(fmt::memory_buffer &buff) const
    {
      return to_string_impl(buff, false);
    }

    native_persistent_string to_string() const
    {
      fmt::memory_buffer buff;
      to_string_impl(buff, false);
      return native_persistent_string{ buff.data(), buff.size() };
    }

    native_persistent_string to_code_string() const
    {
      fmt::memory_buffer buff;
      to_string_impl(buff, true);
      return native_persistent_string{ buff.data(), buff.size() };
    }

    native_hash to_hash() const
    {
      return hash::unordered(&static_cast<parent_type const *>(this)->base);
    }

    /* behavior::countable */
    size_t count() const
    {
      return std::distance(begin, end);
    }

    /* behavior::seqable */
    native_box<parent_type> seq()
    {
      return static_cast<parent_type *>(this);
    }

    native_box<parent_type> fresh_seq() const
    {
      return make_box<parent_type>(coll, begin, end);
    }

    /* behavior::sequenceable */
    obj::persistent_vector_ptr first() const
    {
      auto const pair(*begin);
      return make_box<obj::persistent_vector>(
        runtime::detail::native_persistent_vector{ pair.first, pair.second });
    }

    native_box<parent_type> next() const
    {
      auto n(begin);
      ++n;

      if(n == end)
      {
        return nullptr;
      }

      return make_box<parent_type>(coll, n, end);
    }

    /* behavior::sequenceable_in_place */
    native_box<parent_type> next_in_place()
    {
      ++begin;

      if(begin == end)
      {
        return nullptr;
      }

      return static_cast<parent_type *>(this);
    }

    obj::cons_ptr conj(object_ptr const head)
    {
      return make_box<obj::cons>(head, static_cast<parent_type *>(this));
    }

    object base{ OT };
    object_ptr coll{};
    iterator_type begin{}, end{};
  };
}
