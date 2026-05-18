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
  jit_function::jit_function()
    : object{ obj_type, obj_behaviors }
  {
  }

  jit_function::jit_function(callable_arity_flags const arity_flags)
    : object{ obj_type, obj_behaviors }
    , arity_flags{ arity_flags }
  {
  }

  jit_function::jit_function(object_ref const meta)
    : object{ obj_type, obj_behaviors }
    , meta{ meta }
  {
  }

  void jit_function::to_string(jtl::string_builder &buff) const
  {
    auto const name(meta.get(__rt_ctx->intern_keyword("name").expect_ok()));
    util::format_to(
      buff,
      "#object [{} {} {}]",
      (name.get_type() == object_type::nil ? "unknown" : try_object<persistent_string>(name)->data),
      object_type_str(type),
      this);
  }

  jit_function_ref jit_function::with_meta(object_ref const m)
  {
    auto const new_meta(behavior::detail::validate_meta(m));
    auto const ret{ make_box<jit_function>(*this) };
    ret->meta = new_meta;
    return ret;
  }

  object_ref jit_function::get_meta() const
  {
    return meta;
  }

  object_ref jit_function::call() const
  {
    if(!arity_0)
    {
      throw invalid_arity<0>{ runtime::to_code_string(runtime::detail::untagged(this)) };
    }
    return arity_0(runtime::detail::untagged(this));
  }

  object_ref jit_function::call(object_ref const a1) const
  {
    if(!arity_1)
    {
      throw invalid_arity<1>{ runtime::to_code_string(runtime::detail::untagged(this)) };
    }
    return arity_1(runtime::detail::untagged(this), a1);
  }

  object_ref jit_function::call(object_ref const a1, object_ref const a2) const
  {
    if(!arity_2)
    {
      throw invalid_arity<2>{ runtime::to_code_string(runtime::detail::untagged(this)) };
    }
    return arity_2(runtime::detail::untagged(this), a1, a2);
  }

  object_ref jit_function::call(object_ref const a1, object_ref const a2, object_ref const a3) const
  {
    if(!arity_3)
    {
      throw invalid_arity<3>{ runtime::to_code_string(runtime::detail::untagged(this)) };
    }
    return arity_3(runtime::detail::untagged(this), a1, a2, a3);
  }

  object_ref jit_function::call(object_ref const a1,
                                object_ref const a2,
                                object_ref const a3,
                                object_ref const a4) const
  {
    if(!arity_4)
    {
      throw invalid_arity<4>{ runtime::to_code_string(runtime::detail::untagged(this)) };
    }
    return arity_4(runtime::detail::untagged(this), a1, a2, a3, a4);
  }

  object_ref jit_function::call(object_ref const a1,
                                object_ref const a2,
                                object_ref const a3,
                                object_ref const a4,
                                object_ref const a5) const
  {
    if(!arity_5)
    {
      throw invalid_arity<5>{ runtime::to_code_string(runtime::detail::untagged(this)) };
    }
    return arity_5(runtime::detail::untagged(this), a1, a2, a3, a4, a5);
  }

  object_ref jit_function::call(object_ref const a1,
                                object_ref const a2,
                                object_ref const a3,
                                object_ref const a4,
                                object_ref const a5,
                                object_ref const a6) const
  {
    if(!arity_6)
    {
      throw invalid_arity<6>{ runtime::to_code_string(runtime::detail::untagged(this)) };
    }
    return arity_6(runtime::detail::untagged(this), a1, a2, a3, a4, a5, a6);
  }

  object_ref jit_function::call(object_ref const a1,
                                object_ref const a2,
                                object_ref const a3,
                                object_ref const a4,
                                object_ref const a5,
                                object_ref const a6,
                                object_ref const a7) const
  {
    if(!arity_7)
    {
      throw invalid_arity<7>{ runtime::to_code_string(runtime::detail::untagged(this)) };
    }
    return arity_7(runtime::detail::untagged(this), a1, a2, a3, a4, a5, a6, a7);
  }

  object_ref jit_function::call(object_ref const a1,
                                object_ref const a2,
                                object_ref const a3,
                                object_ref const a4,
                                object_ref const a5,
                                object_ref const a6,
                                object_ref const a7,
                                object_ref const a8) const
  {
    if(!arity_8)
    {
      throw invalid_arity<8>{ runtime::to_code_string(runtime::detail::untagged(this)) };
    }
    return arity_8(runtime::detail::untagged(this), a1, a2, a3, a4, a5, a6, a7, a8);
  }

  object_ref jit_function::call(object_ref const a1,
                                object_ref const a2,
                                object_ref const a3,
                                object_ref const a4,
                                object_ref const a5,
                                object_ref const a6,
                                object_ref const a7,
                                object_ref const a8,
                                object_ref const a9) const
  {
    if(!arity_9)
    {
      throw invalid_arity<9>{ runtime::to_code_string(runtime::detail::untagged(this)) };
    }
    return arity_9(runtime::detail::untagged(this), a1, a2, a3, a4, a5, a6, a7, a8, a9);
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
                                object_ref const a10) const
  {
    if(!arity_10)
    {
      throw invalid_arity<10>{ runtime::to_code_string(runtime::detail::untagged(this)) };
    }
    return arity_10(runtime::detail::untagged(this), a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  }

  callable_arity_flags jit_function::get_arity_flags() const
  {
    return arity_flags;
  }
}
