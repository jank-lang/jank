namespace jank::runtime
{
  obj::transient_sorted_set::static_object(runtime::detail::native_persistent_sorted_set &&d)
    : data{ std::move(d).transient() }
  {
  }

  obj::transient_sorted_set::static_object(runtime::detail::native_persistent_sorted_set const &d)
    : data{ d.transient() }
  {
  }

  obj::transient_sorted_set::static_object(runtime::detail::native_transient_sorted_set &&d)
    : data{ std::move(d) }
  {
  }

  native_bool obj::transient_sorted_set::equal(object const &o) const
  {
    /* Transient equality, in Clojure, is based solely on identity. */
    return &base == &o;
  }

  native_persistent_string obj::transient_sorted_set::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  void obj::transient_sorted_set::to_string(fmt::memory_buffer &buff) const
  {
    auto inserter(std::back_inserter(buff));
    fmt::format_to(inserter, "{}@{}", magic_enum::enum_name(base.type), fmt::ptr(&base));
  }

  native_persistent_string obj::transient_sorted_set::to_code_string() const
  {
    return to_string();
  }

  native_hash obj::transient_sorted_set::to_hash() const
  {
    /* Hash is also based only on identity. Clojure uses default hashCode, which does the same. */
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  size_t obj::transient_sorted_set::count() const
  {
    assert_active();
    return data.size();
  }

  obj::transient_sorted_set_ptr obj::transient_sorted_set::conj_in_place(object_ptr const elem)
  {
    assert_active();
    data.insert_v(elem);
    return this;
  }

  native_box<obj::transient_sorted_set::persistent_type> obj::transient_sorted_set::to_persistent()
  {
    assert_active();
    active = false;
    return make_box<obj::persistent_sorted_set>(data.persistent());
  }

  object_ptr obj::transient_sorted_set::call(object_ptr const elem)
  {
    assert_active();
    auto const found(data.find(elem));
    if(found != data.end())
    {
      return found.get();
    }
    return obj::nil::nil_const();
  }

  object_ptr obj::transient_sorted_set::call(object_ptr const elem, object_ptr const fallback)
  {
    assert_active();
    auto const found(data.find(elem));
    if(found != data.end())
    {
      return found.get();
    }
    return fallback;
  }

  object_ptr obj::transient_sorted_set::get(object_ptr const elem)
  {
    return call(elem);
  }

  object_ptr obj::transient_sorted_set::get(object_ptr const elem, object_ptr const fallback)
  {
    return call(elem, fallback);
  }

  object_ptr obj::transient_sorted_set::get_entry(object_ptr const elem)
  {
    auto const found = call(elem);
    auto const nil(obj::nil::nil_const());
    if(found == nil)
    {
      return nil;
    }

    return make_box<obj::persistent_vector>(std::in_place, found, found);
  }

  native_bool obj::transient_sorted_set::contains(object_ptr const elem) const
  {
    assert_active();
    return data.find(elem) != data.end();
  }

  obj::transient_sorted_set_ptr obj::transient_sorted_set::disjoin_in_place(object_ptr const elem)
  {
    assert_active();
    data.erase_key(elem);
    return this;
  }

  void obj::transient_sorted_set::assert_active() const
  {
    if(!active)
    {
      throw std::runtime_error{ "transient used after it's been made persistent" };
    }
  }
}
