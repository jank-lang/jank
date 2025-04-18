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

  jit_function::jit_function(object_ref const meta)
    : meta{ meta }
  {
  }

  bool jit_function::equal(object const &rhs) const
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
      get(meta.unwrap_or(jank_nil), __rt_ctx->intern_keyword("name").expect_ok()));
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

  uhash jit_function::to_hash() const
  {
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
  }

  jit_function_ref jit_function::with_meta(object_ref const m)
  {
    auto const new_meta(behavior::detail::validate_meta(m));
    meta = new_meta;
    return this;
  }

  object_ref jit_function::call()
  {
    if(!arity_0)
    {
      throw invalid_arity<0>{ runtime::to_string(this_object_ref()) };
    }
    return arity_0();
  }

  object_ref jit_function::call(object_ref const a1)
  {
    if(!arity_1)
    {
      throw invalid_arity<1>{ runtime::to_string(this_object_ref()) };
    }
    return arity_1(a1.data);
  }

  object_ref jit_function::call(object_ref const a1, object_ref const a2)
  {
    if(!arity_2)
    {
      throw invalid_arity<2>{ runtime::to_string(this_object_ref()) };
    }
    return arity_2(a1.data, a2.data);
  }

  object_ref jit_function::call(object_ref const a1, object_ref const a2, object_ref const a3)
  {
    if(!arity_3)
    {
      throw invalid_arity<3>{ runtime::to_string(this_object_ref()) };
    }
    return arity_3(a1.data, a2.data, a3.data);
  }

  object_ref jit_function::call(object_ref const a1,
                                object_ref const a2,
                                object_ref const a3,
                                object_ref const a4)
  {
    if(!arity_4)
    {
      throw invalid_arity<4>{ runtime::to_string(this_object_ref()) };
    }
    return arity_4(a1.data, a2.data, a3.data, a4.data);
  }

  object_ref jit_function::call(object_ref const a1,
                                object_ref const a2,
                                object_ref const a3,
                                object_ref const a4,
                                object_ref const a5)
  {
    if(!arity_5)
    {
      throw invalid_arity<5>{ runtime::to_string(this_object_ref()) };
    }
    return arity_5(a1.data, a2.data, a3.data, a4.data, a5.data);
  }

  object_ref jit_function::call(object_ref const a1,
                                object_ref const a2,
                                object_ref const a3,
                                object_ref const a4,
                                object_ref const a5,
                                object_ref const a6)
  {
    if(!arity_6)
    {
      throw invalid_arity<6>{ runtime::to_string(this_object_ref()) };
    }
    return arity_6(a1.data, a2.data, a3.data, a4.data, a5.data, a6.data);
  }

  object_ref jit_function::call(object_ref const a1,
                                object_ref const a2,
                                object_ref const a3,
                                object_ref const a4,
                                object_ref const a5,
                                object_ref const a6,
                                object_ref const a7)
  {
    if(!arity_7)
    {
      throw invalid_arity<7>{ runtime::to_string(this_object_ref()) };
    }
    return arity_7(a1.data, a2.data, a3.data, a4.data, a5.data, a6.data, a7.data);
  }

  object_ref jit_function::call(object_ref const a1,
                                object_ref const a2,
                                object_ref const a3,
                                object_ref const a4,
                                object_ref const a5,
                                object_ref const a6,
                                object_ref const a7,
                                object_ref const a8)
  {
    if(!arity_8)
    {
      throw invalid_arity<8>{ runtime::to_string(this_object_ref()) };
    }
    return arity_8(a1.data, a2.data, a3.data, a4.data, a5.data, a6.data, a7.data, a8.data);
  }

  object_ref jit_function::call(object_ref const a1,
                                object_ref const a2,
                                object_ref const a3,
                                object_ref const a4,
                                object_ref const a5,
                                object_ref const a6,
                                object_ref const a7,
                                object_ref const a8,
                                object_ref const a9)
  {
    if(!arity_9)
    {
      throw invalid_arity<9>{ runtime::to_string(this_object_ref()) };
    }
    return arity_9(a1.data, a2.data, a3.data, a4.data, a5.data, a6.data, a7.data, a8.data, a9.data);
  }

  object_ref jit_function::call(object_ref const a1,
                                object_ref const a2,
                                object_ref const a3,
                                object_ref const a4,
                                object_ref const a5,
                                object_ref const a6,
                                object_ref const a7,
                                object_ref const a8,
                                object_ref const a9,
                                object_ref const a10)
  {
    if(!arity_10)
    {
      throw invalid_arity<10>{ runtime::to_string(this_object_ref()) };
    }
    return arity_10(a1.data,
                    a2.data,
                    a3.data,
                    a4.data,
                    a5.data,
                    a6.data,
                    a7.data,
                    a8.data,
                    a9.data,
                    a10.data);
  }

  behavior::callable::arity_flag_t jit_function::get_arity_flags() const
  {
    return arity_flags;
  }

  object_ref jit_function::this_object_ref()
  {
    return &this->base;
  }
}
