#include <iostream>

#include <jank/runtime/obj/function.hpp>

namespace jank::runtime::obj
{
  function::function(detail::function_type &&d)
    : data{ std::move(d) }
  { }
  function::function(detail::function_type const &d)
    : data{ d }
  { }

  runtime::detail::box_type<function> function::create(detail::function_type const &d)
  { return make_box<function>(d); }

  runtime::detail::boolean_type function::equal(object const &o) const
  {
    auto const *d(o.as_function());
    return d == this;
  }
  void function::to_string(fmt::memory_buffer &buff) const
  { format_to(std::back_inserter(buff), "function"); }
  runtime::detail::string_type function::to_string() const
  /* TODO: Optimize. */
  { return "function"; }
  runtime::detail::integer_type function::to_hash() const
  { return reinterpret_cast<runtime::detail::integer_type>(this); }
  function const* function::as_function() const
  { return this; }
  behavior::callable const* function::as_callable() const
  { return this; }

  template <size_t N, typename... Args>
  struct build_arity
  { using type = typename build_arity<N - 1, Args..., object_ptr const &>::type; };
  template <typename... Args>
  struct build_arity<0, Args...>
  { using type = object_ptr (Args const&...); };

  template <typename... Args>
  object_ptr apply_function(function const &f, Args &&... args)
  {
    size_t constexpr arg_count{ sizeof...(Args) };
    using arity = typename build_arity<arg_count>::type;
    using function_type = detail::function_type::value_type<arity>;

    auto const * const func_ptr(f.data.template get<function_type>());
    if(!func_ptr)
    {
      /* TODO: Throw error. */
      std::cout << "invalid function arity; expected " << arg_count << std::endl;
      return JANK_NIL;
    }

    return (*func_ptr)(std::forward<Args>(args)...);
  }

  object_ptr function::call() const
  { return apply_function(*this); }
  object_ptr function::call(object_ptr const &arg1) const
  { return apply_function(*this, arg1); }
  object_ptr function::call(object_ptr const &arg1, object_ptr const &arg2) const
  { return apply_function(*this, arg1, arg2); }
  object_ptr function::call(object_ptr const &arg1, object_ptr const &arg2, object_ptr const &arg3) const
  { return apply_function(*this, arg1, arg2, arg3); }
  object_ptr function::call(object_ptr const &arg1, object_ptr const &arg2, object_ptr const &arg3, object_ptr const &arg4) const
  { return apply_function(*this, arg1, arg2, arg3, arg4); }
  object_ptr function::call(object_ptr const &arg1, object_ptr const &arg2, object_ptr const &arg3, object_ptr const &arg4, object_ptr const &arg5) const
  { return apply_function(*this, arg1, arg2, arg3, arg4, arg5); }
  object_ptr function::call(object_ptr const &arg1, object_ptr const &arg2, object_ptr const &arg3, object_ptr const &arg4, object_ptr const &arg5, object_ptr const &arg6) const
  { return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6); }
  object_ptr function::call(object_ptr const &arg1, object_ptr const &arg2, object_ptr const &arg3, object_ptr const &arg4, object_ptr const &arg5, object_ptr const &arg6, object_ptr const &arg7) const
  { return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6, arg7); }
  object_ptr function::call(object_ptr const &arg1, object_ptr const &arg2, object_ptr const &arg3, object_ptr const &arg4, object_ptr const &arg5, object_ptr const &arg6, object_ptr const &arg7, object_ptr const &arg8) const
  { return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); }
  object_ptr function::call(object_ptr const &arg1, object_ptr const &arg2, object_ptr const &arg3, object_ptr const &arg4, object_ptr const &arg5, object_ptr const &arg6, object_ptr const &arg7, object_ptr const &arg8, object_ptr const &arg9) const
  { return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9); }
  object_ptr function::call(object_ptr const &arg1, object_ptr const &arg2, object_ptr const &arg3, object_ptr const &arg4, object_ptr const &arg5, object_ptr const &arg6, object_ptr const &arg7, object_ptr const &arg8, object_ptr const &arg9, object_ptr const &arg10) const
  { return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10); }

  object_ptr function::with_meta(object_ptr const &m) const
  {
    validate_meta(m);
    auto ret(make_box<function>(data));
    ret->meta = m;
    return ret;
  }

  behavior::metadatable const* function::as_metadatable() const
  { return this; }
}
