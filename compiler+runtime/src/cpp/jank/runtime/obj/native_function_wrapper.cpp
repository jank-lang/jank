#include <iostream>

#include <jank/runtime/obj/native_function_wrapper.hpp>

namespace jank::runtime
{
  obj::native_function_wrapper::static_object(obj::detail::function_type &&d)
    : data{ std::move(d) }
  {
  }

  obj::native_function_wrapper::static_object(obj::detail::function_type const &d)
    : data{ d }
  {
  }

  native_bool obj::native_function_wrapper::equal(object const &o) const
  {
    return &base == &o;
  }

  void obj::native_function_wrapper::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff),
                   "{}@{}",
                   magic_enum::enum_name(base.type),
                   fmt::ptr(&base));
  }

  native_persistent_string obj::native_function_wrapper::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_persistent_string obj::native_function_wrapper::to_code_string() const
  {
    return to_string();
  }

  native_hash obj::native_function_wrapper::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  template <size_t N, typename... Args>
  struct build_arity
  {
    using type = typename build_arity<N - 1, Args..., object_ptr>::type;
  };

  template <typename... Args>
  struct build_arity<0, Args...>
  {
    using type = object_ptr(Args...);
  };

  template <typename... Args>
  object_ptr apply_function(obj::native_function_wrapper const &f, Args &&...args)
  {
    constexpr size_t arg_count{ sizeof...(Args) };
    using arity = typename build_arity<arg_count>::type;
    using function_type = obj::detail::function_type::value_type<arity>;

    auto const * const func_ptr(f.data.template get<function_type>());
    if(!func_ptr)
    {
      throw std::runtime_error{ fmt::format("invalid function arity; tried {}", arg_count) };
    }

    return (*func_ptr)(std::forward<Args>(args)...);
  }

  object_ptr obj::native_function_wrapper::call()
  {
    return apply_function(*this);
  }

  object_ptr obj::native_function_wrapper::call(object_ptr arg1)
  {
    return apply_function(*this, arg1);
  }

  object_ptr obj::native_function_wrapper::call(object_ptr arg1, object_ptr arg2)
  {
    return apply_function(*this, arg1, arg2);
  }

  object_ptr obj::native_function_wrapper::call(object_ptr arg1, object_ptr arg2, object_ptr arg3)
  {
    return apply_function(*this, arg1, arg2, arg3);
  }

  object_ptr obj::native_function_wrapper::call(object_ptr arg1,
                                                object_ptr arg2,
                                                object_ptr arg3,
                                                object_ptr arg4)
  {
    return apply_function(*this, arg1, arg2, arg3, arg4);
  }

  object_ptr obj::native_function_wrapper::call(object_ptr arg1,
                                                object_ptr arg2,
                                                object_ptr arg3,
                                                object_ptr arg4,
                                                object_ptr arg5)
  {
    return apply_function(*this, arg1, arg2, arg3, arg4, arg5);
  }

  object_ptr obj::native_function_wrapper::call(object_ptr arg1,
                                                object_ptr arg2,
                                                object_ptr arg3,
                                                object_ptr arg4,
                                                object_ptr arg5,
                                                object_ptr arg6)
  {
    return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6);
  }

  object_ptr obj::native_function_wrapper::call(object_ptr arg1,
                                                object_ptr arg2,
                                                object_ptr arg3,
                                                object_ptr arg4,
                                                object_ptr arg5,
                                                object_ptr arg6,
                                                object_ptr arg7)
  {
    return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
  }

  object_ptr obj::native_function_wrapper::call(object_ptr arg1,
                                                object_ptr arg2,
                                                object_ptr arg3,
                                                object_ptr arg4,
                                                object_ptr arg5,
                                                object_ptr arg6,
                                                object_ptr arg7,
                                                object_ptr arg8)
  {
    return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
  }

  object_ptr obj::native_function_wrapper::call(object_ptr arg1,
                                                object_ptr arg2,
                                                object_ptr arg3,
                                                object_ptr arg4,
                                                object_ptr arg5,
                                                object_ptr arg6,
                                                object_ptr arg7,
                                                object_ptr arg8,
                                                object_ptr arg9)
  {
    return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
  }

  object_ptr obj::native_function_wrapper::call(object_ptr arg1,
                                                object_ptr arg2,
                                                object_ptr arg3,
                                                object_ptr arg4,
                                                object_ptr arg5,
                                                object_ptr arg6,
                                                object_ptr arg7,
                                                object_ptr arg8,
                                                object_ptr arg9,
                                                object_ptr arg10)
  {
    return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
  }

  obj::native_function_wrapper_ptr obj::native_function_wrapper::with_meta(object_ptr const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<obj::native_function_wrapper>(data));
    ret->meta = meta;
    return ret;
  }

  object_ptr obj::native_function_wrapper::this_object_ptr()
  {
    return &this->base;
  }
}
