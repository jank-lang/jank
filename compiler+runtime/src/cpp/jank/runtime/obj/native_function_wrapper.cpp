#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  native_function_wrapper::native_function_wrapper(detail::function_type &&d)
    : data{ std::move(d) }
  {
  }

  native_function_wrapper::native_function_wrapper(detail::function_type const &d)
    : data{ d }
  {
  }

  bool native_function_wrapper::equal(object const &o) const
  {
    return &base == &o;
  }

  void native_function_wrapper::to_string(jtl::string_builder &buff) const
  {
    auto const name(get(meta.unwrap_or(jank_nil()), __rt_ctx->intern_keyword("name").expect_ok()));
    util::format_to(buff,
                    "#object [{} {} {}]",
                    (name.is_nil() ? "unknown" : try_object<persistent_string>(name)->data),
                    object_type_str(base.type),
                    &base);
  }

  jtl::immutable_string native_function_wrapper::to_string() const
  {
    jtl::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  jtl::immutable_string native_function_wrapper::to_code_string() const
  {
    return to_string();
  }

  uhash native_function_wrapper::to_hash() const
  {
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
  }

  template <usize N, typename... Args>
  struct build_arity
  {
    using type = typename build_arity<N - 1, Args..., object_ref const>::type;
  };

  template <typename... Args>
  struct build_arity<0, Args...>
  {
    using type = object_ref(Args...);
  };

  template <typename... Args>
  static object_ref apply_function(native_function_wrapper const &f, Args &&...args)
  {
    constexpr usize arg_count{ sizeof...(Args) };
    using arity = typename build_arity<arg_count>::type;
    using function_type = detail::function_type::value_type<arity>;

    auto const * const func_ptr(f.data.template get<function_type>());
    if(!func_ptr)
    {
      jtl::immutable_string name{ f.to_string() };
      if(f.meta.is_some())
      {
        auto const name_kw(__rt_ctx->intern_keyword("name").expect_ok());
        auto const name_meta(runtime::get(f.meta.unwrap(), name_kw));
        if(name_meta != jank_nil())
        {
          name = to_string(name_meta);
        }
      }
      throw invalid_arity<sizeof...(Args)>{ name };
    }

    return (*func_ptr)(std::forward<Args>(args)...);
  }

  object_ref native_function_wrapper::call()
  {
    return apply_function(*this);
  }

  object_ref native_function_wrapper::call(object_ref const arg1)
  {
    return apply_function(*this, arg1);
  }

  object_ref native_function_wrapper::call(object_ref const arg1, object_ref const arg2)
  {
    return apply_function(*this, arg1, arg2);
  }

  object_ref
  native_function_wrapper::call(object_ref const arg1, object_ref const arg2, object_ref const arg3)
  {
    return apply_function(*this, arg1, arg2, arg3);
  }

  object_ref native_function_wrapper::call(object_ref const arg1,
                                           object_ref const arg2,
                                           object_ref const arg3,
                                           object_ref const arg4)
  {
    return apply_function(*this, arg1, arg2, arg3, arg4);
  }

  object_ref native_function_wrapper::call(object_ref const arg1,
                                           object_ref const arg2,
                                           object_ref const arg3,
                                           object_ref const arg4,
                                           object_ref const arg5)
  {
    return apply_function(*this, arg1, arg2, arg3, arg4, arg5);
  }

  object_ref native_function_wrapper::call(object_ref const arg1,
                                           object_ref const arg2,
                                           object_ref const arg3,
                                           object_ref const arg4,
                                           object_ref const arg5,
                                           object_ref const arg6)
  {
    return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6);
  }

  object_ref native_function_wrapper::call(object_ref const arg1,
                                           object_ref const arg2,
                                           object_ref const arg3,
                                           object_ref const arg4,
                                           object_ref const arg5,
                                           object_ref const arg6,
                                           object_ref const arg7)
  {
    return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
  }

  object_ref native_function_wrapper::call(object_ref const arg1,
                                           object_ref const arg2,
                                           object_ref const arg3,
                                           object_ref const arg4,
                                           object_ref const arg5,
                                           object_ref const arg6,
                                           object_ref const arg7,
                                           object_ref const arg8)
  {
    return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
  }

  object_ref native_function_wrapper::call(object_ref const arg1,
                                           object_ref const arg2,
                                           object_ref const arg3,
                                           object_ref const arg4,
                                           object_ref const arg5,
                                           object_ref const arg6,
                                           object_ref const arg7,
                                           object_ref const arg8,
                                           object_ref const arg9)
  {
    return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
  }

  object_ref native_function_wrapper::call(object_ref const arg1,
                                           object_ref const arg2,
                                           object_ref const arg3,
                                           object_ref const arg4,
                                           object_ref const arg5,
                                           object_ref const arg6,
                                           object_ref const arg7,
                                           object_ref const arg8,
                                           object_ref const arg9,
                                           object_ref const arg10)
  {
    return apply_function(*this, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
  }

  native_function_wrapper_ref native_function_wrapper::with_meta(object_ref const m) const
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<native_function_wrapper>(data));
    ret->meta = meta;
    return ret;
  }

  object_ref native_function_wrapper::this_object_ref()
  {
    return &this->base;
  }
}
