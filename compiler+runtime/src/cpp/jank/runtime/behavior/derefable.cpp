namespace jank::runtime::behavior
{
  object_ptr deref(object_ptr const ref)
  {
    return visit_object(
      [=](auto const typed_ref) -> object_ptr {
        using T = typename decltype(typed_ref)::value_type;

        if constexpr(behavior::derefable<T>)
        {
          return typed_ref->deref();
        }
        else
        {
          throw std::runtime_error{ fmt::format("not derefable: {}", typed_ref->to_string()) };
        }
      },
      ref);
  }
}
