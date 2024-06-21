namespace jank::runtime
{
  obj::transient_vector::static_object(runtime::detail::native_persistent_vector &&d)
    : data{ std::move(d).transient() }
  {
  }

  obj::transient_vector::static_object(runtime::detail::native_persistent_vector const &d)
    : data{ d.transient() }
  {
  }

  obj::transient_vector::static_object(runtime::detail::native_transient_vector &&d)
    : data{ std::move(d) }
  {
  }

  native_bool obj::transient_vector::equal(object const &o) const
  {
    /* Transient equality, in Clojure, is based solely on identity. */
    return &base == &o;
  }

  native_persistent_string obj::transient_vector::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  void obj::transient_vector::to_string(fmt::memory_buffer &buff) const
  {
    auto inserter(std::back_inserter(buff));
    fmt::format_to(inserter, "{}@{}", magic_enum::enum_name(base.type), fmt::ptr(&base));
  }

  native_hash obj::transient_vector::to_hash() const
  {
    /* Hash is also based only on identity. Clojure uses default hashCode, which does the same. */
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  size_t obj::transient_vector::count() const
  {
    assert_active();
    return data.size();
  }

  obj::transient_vector_ptr obj::transient_vector::cons_in_place(object_ptr const head)
  {
    assert_active();
    data.push_back(head);
    return this;
  }

  native_box<obj::transient_vector::persistent_type> obj::transient_vector::to_persistent()
  {
    assert_active();
    active = false;
    return make_box<obj::persistent_vector>(data.persistent());
  }

  object_ptr obj::transient_vector::call(object_ptr const idx)
  {
    assert_active();
    if(idx->type == object_type::integer)
    {
      auto const i(expect_object<obj::integer>(idx)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        throw std::runtime_error{
          fmt::format("Index out of bound; index = {}, count = {}", i, count())
        };
      }

      return data[i];
    }
    else
    {
      throw std::runtime_error{ fmt::format("key must be an integer; found {}",
                                            runtime::detail::to_string(idx)) };
    }
  }

  object_ptr obj::transient_vector::get(object_ptr const idx) const
  {
    assert_active();
    if(idx->type == object_type::integer)
    {
      auto const i(expect_object<obj::integer>(idx)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return obj::nil::nil_const();
      }

      return data[i];
    }
    else
    {
      throw std::runtime_error{ fmt::format("key must be an integer; found {}",
                                            runtime::detail::to_string(idx)) };
    }
  }

  object_ptr obj::transient_vector::get(object_ptr const idx, object_ptr const fallback) const
  {
    assert_active();
    if(idx->type == object_type::integer)
    {
      auto const i(expect_object<obj::integer>(idx)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return fallback;
      }

      return data[i];
    }
    else
    {
      throw std::runtime_error{ fmt::format("key must be an integer; found {}",
                                            runtime::detail::to_string(idx)) };
    }
  }

  object_ptr obj::transient_vector::get_entry(object_ptr const idx) const
  {
    if(idx->type == object_type::integer)
    {
      auto const i(expect_object<obj::integer>(idx)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return obj::nil::nil_const();
      }
      /* TODO: Map entry type? */
      return make_box<obj::persistent_vector>(std::in_place, idx, data[i]);
    }
    else
    {
      throw std::runtime_error{ fmt::format("get_entry on a vector must be an integer; found {}",
                                            runtime::detail::to_string(idx)) };
    }
  }

  native_bool obj::transient_vector::contains(object_ptr const elem) const
  {
    if(elem->type == object_type::integer)
    {
      auto const i(expect_object<obj::integer>(elem)->data);
      return i >= 0 && static_cast<size_t>(i) < data.size();
    }
    else
    {
      return false;
    }
  }

  obj::transient_vector_ptr obj::transient_vector::pop_in_place()
  {
    assert_active();
    if(data.empty())
    {
      throw std::runtime_error{ "Can't pop empty vector" };
    }

    data.take(data.size() - 1);
    return this;
  }

  void obj::transient_vector::assert_active() const
  {
    if(!active)
    {
      throw std::runtime_error{ "transient used after it's been made persistent" };
    }
  }
}
