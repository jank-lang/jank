#pragma once

namespace jank::runtime
{
  native_bool is_map(object_ptr o);

  namespace behavior::detail
  {
    object_ptr validate_meta(object_ptr const m);
  }
}

namespace jank::runtime::obj::detail
{
  /* Array maps and hash maps share a lot of common code, so we have a common base.
   * No virtual fns are used, so this structure won't survive release optimizations. */
  template <object_type OT, object_type ST, typename V>
  struct base_persistent_map : gc
  {
    using value_type = V;
    using parent_type = static_object<OT>;
    using sequence_type = static_object<ST>;

    static constexpr native_bool pointer_free{ false };

    base_persistent_map() = default;

    base_persistent_map(native_box<obj::persistent_array_map> const meta)
      : meta{ meta }
    {
    }

    /* behavior::object_like */
    native_bool equal(object const &o) const
    {
      object_ptr p{ const_cast<object *>(&o) };

      if(!is_map(p))
      {
        return false;
      }

      /* TODO: Is this enough? Value comparison may be needed. */
      return to_hash() == hash::visit(p);
    }

    static void to_string_impl(typename V::const_iterator const &begin,
                               typename V::const_iterator const &end,
                               fmt::memory_buffer &buff,
                               native_bool to_code = false)
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

    void to_string(fmt::memory_buffer &buff) const
    {
      to_string_impl(static_cast<parent_type const *>(this)->data.begin(),
                     static_cast<parent_type const *>(this)->data.end(),
                     buff);
    }

    native_persistent_string to_string() const
    {
      fmt::memory_buffer buff;
      to_string_impl(static_cast<parent_type const *>(this)->data.begin(),
                     static_cast<parent_type const *>(this)->data.end(),
                     buff);
      return native_persistent_string{ buff.data(), buff.size() };
    }

    native_persistent_string to_code_string() const
    {
      fmt::memory_buffer buff;
      to_string_impl(static_cast<parent_type const *>(this)->data.begin(),
                     static_cast<parent_type const *>(this)->data.end(),
                     buff,
                     true);
      return native_persistent_string{ buff.data(), buff.size() };
    }

    native_hash to_hash() const
    {
      if(hash)
      {
        return hash;
      }

      return hash = hash::unordered(static_cast<parent_type const *>(this)->data.begin(),
                                    static_cast<parent_type const *>(this)->data.end());
    }

    /* behavior::seqable */
    native_box<sequence_type> seq() const
    {
      if(static_cast<parent_type const *>(this)->data.empty())
      {
        return nullptr;
      }
      return make_box<sequence_type>(static_cast<parent_type const *>(this),
                                     static_cast<parent_type const *>(this)->data.begin(),
                                     static_cast<parent_type const *>(this)->data.end());
    }

    native_box<sequence_type> fresh_seq() const
    {
      if(static_cast<parent_type const *>(this)->data.empty())
      {
        return nullptr;
      }
      return make_box<sequence_type>(static_cast<parent_type const *>(this),
                                     static_cast<parent_type const *>(this)->data.begin(),
                                     static_cast<parent_type const *>(this)->data.end());
    }

    /* behavior::countable */
    size_t count() const
    {
      return static_cast<parent_type const *>(this)->data.size();
    }

    /* behavior::metadatable */
    native_box<parent_type> with_meta(object_ptr const m) const
    {
      auto const meta(behavior::detail::validate_meta(m));
      auto ret(make_box<parent_type>(static_cast<parent_type const *>(this)->data));
      ret->meta = meta;
      return ret;
    }

    object base{ OT };
    option<object_ptr> meta;
    mutable native_hash hash{};
  };
}
