#include <jank/runtime/obj/multi_function.hpp>

namespace jank::runtime
{
  obj::multi_function::static_object(object_ptr const name,
                                     object_ptr const dispatch,
                                     object_ptr const default_,
                                     object_ptr const hierarchy)
    : dispatch{ runtime::behavior::to_callable(dispatch) }
    , default_dispatch_value{ default_ }
    , hierarchy{ hierarchy }
    , name{ (name->type == object_type::symbol
               ? expect_object<obj::symbol>(name)
               : throw std::runtime_error{
                   fmt::format("invalid multifn name: {}", runtime::detail::to_string(name)) }) }
  {
  }

  native_bool obj::multi_function::equal(object const &rhs) const
  {
    return &base == &rhs;
  }

  native_persistent_string obj::multi_function::to_string()
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  void obj::multi_function::to_string(fmt::memory_buffer &buff)
  {
    fmt::format_to(std::back_inserter(buff),
                   "{} ({}@{})",
                   name->to_string(),
                   magic_enum::enum_name(base.type),
                   fmt::ptr(&base));
  }

  native_hash obj::multi_function::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  object_ptr obj::multi_function::call() const
  {
    return obj::nil::nil_const();
  }

  object_ptr obj::multi_function::call(object_ptr const a1) const
  {
    return dispatch->call(a1);
  }

  object_ptr obj::multi_function::call(object_ptr, object_ptr) const
  {
    return obj::nil::nil_const();
  }

  object_ptr obj::multi_function::call(object_ptr, object_ptr, object_ptr) const
  {
    return obj::nil::nil_const();
  }

  object_ptr obj::multi_function::call(object_ptr, object_ptr, object_ptr, object_ptr) const
  {
    return obj::nil::nil_const();
  }

  object_ptr
  obj::multi_function::call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const
  {
    return obj::nil::nil_const();
  }

  object_ptr
  obj::multi_function::call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr)
    const
  {
    return obj::nil::nil_const();
  }

  object_ptr obj::multi_function::call(object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr) const
  {
    return obj::nil::nil_const();
  }

  object_ptr obj::multi_function::call(object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr) const
  {
    return obj::nil::nil_const();
  }

  object_ptr obj::multi_function::call(object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr) const
  {
    return obj::nil::nil_const();
  }

  object_ptr obj::multi_function::call(object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr,
                                       object_ptr) const
  {
    return obj::nil::nil_const();
  }

  object_ptr obj::multi_function::this_object_ptr() const
  {
    return &const_cast<obj::multi_function *>(this)->base;
  }
}
