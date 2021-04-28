#include <iostream>

#include <prelude/fn.hpp>

namespace jank
{
  object_ptr callable::call() const
  { throw invalid_arity<0>{}; }
  object_ptr callable::call(object_ptr const&) const
  { throw invalid_arity<1>{}; }
  object_ptr callable::call(object_ptr const&, object_ptr const&) const
  { throw invalid_arity<2>{}; }
  object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&) const
  { throw invalid_arity<3>{}; }
  object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const
  { throw invalid_arity<4>{}; }
  object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const
  { throw invalid_arity<5>{}; }
  object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const
  { throw invalid_arity<6>{}; }
  object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const
  { throw invalid_arity<7>{}; }
  object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const
  { throw invalid_arity<8>{}; }
  object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const
  { throw invalid_arity<9>{}; }
  object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const
  { throw invalid_arity<10>{}; }

  detail::boolean_type function::equal(object const &o) const
  {
    auto const *d(dynamic_cast<function const*>(&o));
    return d == this;
  }
  detail::string_type function::to_string() const
  /* TODO: Optimize. */
  { return "<function>"; }
  detail::integer_type function::to_hash() const
  { return reinterpret_cast<detail::integer_type>(this); }

  template <size_t N, typename... Args>
  struct build_arity
  { using type = typename build_arity<N - 1, Args..., object_ptr>::type; };
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
}
