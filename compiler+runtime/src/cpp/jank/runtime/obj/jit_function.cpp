#include <jank/runtime/obj/jit_function.hpp>

namespace jank::runtime
{
  obj::jit_function::static_object(arity_flag_t const arity_flags)
    : arity_flags{ arity_flags }
  {
  }

  obj::jit_function::static_object(object_ptr const meta)
    : meta{ meta }
  {
  }

  native_bool obj::jit_function::equal(object const &rhs) const
  {
    return &base == &rhs;
  }

  native_persistent_string obj::jit_function::to_string()
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  void obj::jit_function::to_string(fmt::memory_buffer &buff)
  {
    auto const name(
      get(meta.unwrap_or(obj::nil::nil_const()), __rt_ctx->intern_keyword("name").expect_ok()));
    fmt::format_to(std::back_inserter(buff),
                   "{} ({}@{})",
                   (name->type == object_type::nil
                      ? "unknown"
                      : expect_object<obj::persistent_string>(name)->data),
                   magic_enum::enum_name(base.type),
                   fmt::ptr(&base));
  }

  native_persistent_string obj::jit_function::to_code_string()
  {
    return to_string();
  }

  native_hash obj::jit_function::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  obj::jit_function_ptr obj::jit_function::with_meta(object_ptr const m)
  {
    auto const new_meta(behavior::detail::validate_meta(m));
    meta = new_meta;
    return this;
  }

  object_ptr obj::jit_function::call()
  {
    if(!arity_0)
    {
      throw invalid_arity<0>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_0();
  }

  object_ptr obj::jit_function::call(object_ptr const a1)
  {
    if(!arity_1)
    {
      throw invalid_arity<1>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_1(a1);
  }

  object_ptr obj::jit_function::call(object_ptr const a1, object_ptr const a2)
  {
    if(!arity_2)
    {
      throw invalid_arity<2>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_2(a1, a2);
  }

  object_ptr obj::jit_function::call(object_ptr const a1, object_ptr const a2, object_ptr const a3)
  {
    if(!arity_3)
    {
      throw invalid_arity<3>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_3(a1, a2, a3);
  }

  object_ptr obj::jit_function::call(object_ptr const a1,
                                     object_ptr const a2,
                                     object_ptr const a3,
                                     object_ptr const a4)
  {
    if(!arity_4)
    {
      throw invalid_arity<4>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_4(a1, a2, a3, a4);
  }

  object_ptr obj::jit_function::call(object_ptr const a1,
                                     object_ptr const a2,
                                     object_ptr const a3,
                                     object_ptr const a4,
                                     object_ptr const a5)
  {
    if(!arity_5)
    {
      throw invalid_arity<5>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_5(a1, a2, a3, a4, a5);
  }

  object_ptr obj::jit_function::call(object_ptr const a1,
                                     object_ptr const a2,
                                     object_ptr const a3,
                                     object_ptr const a4,
                                     object_ptr const a5,
                                     object_ptr const a6)
  {
    if(!arity_6)
    {
      throw invalid_arity<6>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_6(a1, a2, a3, a4, a5, a6);
  }

  object_ptr obj::jit_function::call(object_ptr const a1,
                                     object_ptr const a2,
                                     object_ptr const a3,
                                     object_ptr const a4,
                                     object_ptr const a5,
                                     object_ptr const a6,
                                     object_ptr const a7)
  {
    if(!arity_7)
    {
      throw invalid_arity<7>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_7(a1, a2, a3, a4, a5, a6, a7);
  }

  object_ptr obj::jit_function::call(object_ptr const a1,
                                     object_ptr const a2,
                                     object_ptr const a3,
                                     object_ptr const a4,
                                     object_ptr const a5,
                                     object_ptr const a6,
                                     object_ptr const a7,
                                     object_ptr const a8)
  {
    if(!arity_8)
    {
      throw invalid_arity<8>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_8(a1, a2, a3, a4, a5, a6, a7, a8);
  }

  object_ptr obj::jit_function::call(object_ptr const a1,
                                     object_ptr const a2,
                                     object_ptr const a3,
                                     object_ptr const a4,
                                     object_ptr const a5,
                                     object_ptr const a6,
                                     object_ptr const a7,
                                     object_ptr const a8,
                                     object_ptr const a9)
  {
    if(!arity_9)
    {
      throw invalid_arity<9>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_9(a1, a2, a3, a4, a5, a6, a7, a8, a9);
  }

  object_ptr obj::jit_function::call(object_ptr const a1,
                                     object_ptr const a2,
                                     object_ptr const a3,
                                     object_ptr const a4,
                                     object_ptr const a5,
                                     object_ptr const a6,
                                     object_ptr const a7,
                                     object_ptr const a8,
                                     object_ptr const a9,
                                     object_ptr const a10)
  {
    if(!arity_10)
    {
      throw invalid_arity<10>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  }

  behavior::callable::arity_flag_t obj::jit_function::get_arity_flags() const
  {
    return arity_flags;
  }

  object_ptr obj::jit_function::this_object_ptr()
  {
    return &this->base;
  }
}
