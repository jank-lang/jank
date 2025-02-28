#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/obj/jit_closure.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/seq.hpp>

namespace jank::runtime::obj
{
  jit_closure::jit_closure(arity_flag_t const arity_flags, void * const context)
    : context{ context }
    , arity_flags{ arity_flags }
  {
  }

  jit_closure::jit_closure(object_ptr const meta)
    : meta{ meta }
  {
  }

  native_bool jit_closure::equal(object const &rhs) const
  {
    return &base == &rhs;
  }

  native_persistent_string jit_closure::to_string()
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void jit_closure::to_string(util::string_builder &buff)
  {
    auto const name(
      get(meta.unwrap_or(nil::nil_const()), __rt_ctx->intern_keyword("name").expect_ok()));
    fmt::format_to(
      std::back_inserter(buff),
      "{} ({}@{})",
      (name->type == object_type::nil ? "unknown" : expect_object<persistent_string>(name)->data),
      object_type_str(base.type),
      fmt::ptr(&base));
  }

  native_persistent_string jit_closure::to_code_string()
  {
    return to_string();
  }

  native_hash jit_closure::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  jit_closure_ptr jit_closure::with_meta(object_ptr const m)
  {
    auto const new_meta(behavior::detail::validate_meta(m));
    meta = new_meta;
    return this;
  }

  object_ptr jit_closure::call()
  {
    if(!arity_0)
    {
      throw invalid_arity<0>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_0(context);
  }

  object_ptr jit_closure::call(object_ptr const a1)
  {
    if(!arity_1)
    {
      throw invalid_arity<1>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_1(context, a1);
  }

  object_ptr jit_closure::call(object_ptr const a1, object_ptr const a2)
  {
    if(!arity_2)
    {
      throw invalid_arity<2>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_2(context, a1, a2);
  }

  object_ptr jit_closure::call(object_ptr const a1, object_ptr const a2, object_ptr const a3)
  {
    if(!arity_3)
    {
      throw invalid_arity<3>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_3(context, a1, a2, a3);
  }

  object_ptr jit_closure::call(object_ptr const a1,
                               object_ptr const a2,
                               object_ptr const a3,
                               object_ptr const a4)
  {
    if(!arity_4)
    {
      throw invalid_arity<4>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_4(context, a1, a2, a3, a4);
  }

  object_ptr jit_closure::call(object_ptr const a1,
                               object_ptr const a2,
                               object_ptr const a3,
                               object_ptr const a4,
                               object_ptr const a5)
  {
    if(!arity_5)
    {
      throw invalid_arity<5>{ runtime::to_string(this_object_ptr()) };
    }
    return arity_5(context, a1, a2, a3, a4, a5);
  }

  object_ptr jit_closure::call(object_ptr const a1,
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
    return arity_6(context, a1, a2, a3, a4, a5, a6);
  }

  object_ptr jit_closure::call(object_ptr const a1,
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
    return arity_7(context, a1, a2, a3, a4, a5, a6, a7);
  }

  object_ptr jit_closure::call(object_ptr const a1,
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
    return arity_8(context, a1, a2, a3, a4, a5, a6, a7, a8);
  }

  object_ptr jit_closure::call(object_ptr const a1,
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
    return arity_9(context, a1, a2, a3, a4, a5, a6, a7, a8, a9);
  }

  object_ptr jit_closure::call(object_ptr const a1,
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
    return arity_10(context, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  }

  behavior::callable::arity_flag_t jit_closure::get_arity_flags() const
  {
    return arity_flags;
  }

  object_ptr jit_closure::this_object_ptr()
  {
    return &this->base;
  }
}
