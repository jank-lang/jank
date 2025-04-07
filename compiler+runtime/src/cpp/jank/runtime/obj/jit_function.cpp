#include <jank/runtime/obj/jit_function.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  jit_function::jit_function(arity_flag_t const arity_flags)
    : arity_flags{ arity_flags }
  {
  }

  jit_function::jit_function(object_ptr const meta)
    : meta{ meta }
  {
  }

  native_bool jit_function::equal(object const &rhs) const
  {
    return &base == &rhs;
  }

  jtl::immutable_string jit_function::to_string()
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void jit_function::to_string(util::string_builder &buff)
  {
    auto const name(
      get(meta.unwrap_or(nil::nil_const()), __rt_ctx->intern_keyword("name").expect_ok()));
    util::format_to(
      buff,
      "{} ({}@{})",
      (name->type == object_type::nil ? "unknown" : expect_object<persistent_string>(name)->data),
      object_type_str(base.type),
      &base);
  }

  jtl::immutable_string jit_function::to_code_string()
  {
    return to_string();
  }

  native_hash jit_function::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  jit_function_ref jit_function::with_meta(object_ptr const m)
  {
    auto const new_meta(behavior::detail::validate_meta(m));
    meta = new_meta;
    return this;
  }

  object_ptr jit_function::call()
  {
    if(!arity_0)
    {
      throw invalid_arity<0>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_0();
  }

  object_ptr jit_function::call(object_ptr const a1)
  {
    if(!arity_1)
    {
      throw invalid_arity<1>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_1(a1);
  }

  object_ptr jit_function::call(object_ptr const a1, object_ptr const a2)
  {
    if(!arity_2)
    {
      throw invalid_arity<2>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_2(a1, a2);
  }

  object_ptr jit_function::call(object_ptr const a1, object_ptr const a2, object_ptr const a3)
  {
    if(!arity_3)
    {
      throw invalid_arity<3>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_3(a1, a2, a3);
  }

  object_ptr jit_function::call(object_ptr const a1,
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

  object_ptr jit_function::call(object_ptr const a1,
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

  object_ptr jit_function::call(object_ptr const a1,
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

  object_ptr jit_function::call(object_ptr const a1,
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

  object_ptr jit_function::call(object_ptr const a1,
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

  object_ptr jit_function::call(object_ptr const a1,
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

  object_ptr jit_function::call(object_ptr const a1,
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

  behavior::callable::arity_flag_t jit_function::get_arity_flags() const
  {
    return arity_flags;
  }

  object_ptr jit_function::this_object_ptr()
  {
    return &this->base;
  }
}
