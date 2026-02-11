#include <jank/runtime/obj/jit_closure.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  jit_closure::jit_closure()
    : object{ obj_type, obj_behaviors }
  {
  }

  jit_closure::jit_closure(callable_arity_flags const arity_flags, void * const context)
    : object{ obj_type, obj_behaviors }
    , context{ context }
    , arity_flags{ arity_flags }
  {
  }

  jit_closure::jit_closure(object_ref const meta)
    : object{ obj_type, obj_behaviors }
    , meta{ meta }
  {
  }

  void jit_closure::to_string(jtl::string_builder &buff) const
  {
    auto const name(get(meta, __rt_ctx->intern_keyword("name").expect_ok()));
    util::format_to(
      buff,
      "#object [{} {} {}]",
      (name->type == object_type::nil ? "unknown" : try_object<persistent_string>(name)->data),
      object_type_str(type),
      this);
  }

  jit_closure_ref jit_closure::with_meta(object_ref const m)
  {
    auto const new_meta(behavior::detail::validate_meta(m));
    auto const ret{ make_box<jit_closure>(*this) };
    ret->meta = new_meta;
    return ret;
  }

  object_ref jit_closure::get_meta() const
  {
    return meta;
  }

  object_ref jit_closure::call() const
  {
    if(!arity_0)
    {
      throw invalid_arity<0>{ runtime::to_string(this) };
    }
    return arity_0(const_cast<jit_closure *>(this));
  }

  object_ref jit_closure::call(object_ref const a1) const
  {
    if(!arity_1)
    {
      throw invalid_arity<1>{ runtime::to_string(this) };
    }
    return arity_1(const_cast<jit_closure *>(this), a1.data);
  }

  object_ref jit_closure::call(object_ref const a1, object_ref const a2) const
  {
    if(!arity_2)
    {
      throw invalid_arity<2>{ runtime::to_string(this) };
    }
    return arity_2(const_cast<jit_closure *>(this), a1.data, a2.data);
  }

  object_ref jit_closure::call(object_ref const a1, object_ref const a2, object_ref const a3) const
  {
    if(!arity_3)
    {
      throw invalid_arity<3>{ runtime::to_string(this) };
    }
    return arity_3(const_cast<jit_closure *>(this), a1.data, a2.data, a3.data);
  }

  object_ref jit_closure::call(object_ref const a1,
                               object_ref const a2,
                               object_ref const a3,
                               object_ref const a4) const
  {
    if(!arity_4)
    {
      throw invalid_arity<4>{ runtime::to_string(this) };
    }
    return arity_4(const_cast<jit_closure *>(this), a1.data, a2.data, a3.data, a4.data);
  }

  object_ref jit_closure::call(object_ref const a1,
                               object_ref const a2,
                               object_ref const a3,
                               object_ref const a4,
                               object_ref const a5) const
  {
    if(!arity_5)
    {
      throw invalid_arity<5>{ runtime::to_string(this) };
    }
    return arity_5(const_cast<jit_closure *>(this), a1.data, a2.data, a3.data, a4.data, a5.data);
  }

  object_ref jit_closure::call(object_ref const a1,
                               object_ref const a2,
                               object_ref const a3,
                               object_ref const a4,
                               object_ref const a5,
                               object_ref const a6) const
  {
    if(!arity_6)
    {
      throw invalid_arity<6>{ runtime::to_string(this) };
    }
    return arity_6(const_cast<jit_closure *>(this),
                   a1.data,
                   a2.data,
                   a3.data,
                   a4.data,
                   a5.data,
                   a6.data);
  }

  object_ref jit_closure::call(object_ref const a1,
                               object_ref const a2,
                               object_ref const a3,
                               object_ref const a4,
                               object_ref const a5,
                               object_ref const a6,
                               object_ref const a7) const
  {
    if(!arity_7)
    {
      throw invalid_arity<7>{ runtime::to_string(this) };
    }
    return arity_7(const_cast<jit_closure *>(this),
                   a1.data,
                   a2.data,
                   a3.data,
                   a4.data,
                   a5.data,
                   a6.data,
                   a7.data);
  }

  object_ref jit_closure::call(object_ref const a1,
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
      throw invalid_arity<8>{ runtime::to_string(this) };
    }
    return arity_8(const_cast<jit_closure *>(this),
                   a1.data,
                   a2.data,
                   a3.data,
                   a4.data,
                   a5.data,
                   a6.data,
                   a7.data,
                   a8.data);
  }

  object_ref jit_closure::call(object_ref const a1,
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
      throw invalid_arity<9>{ runtime::to_string(this) };
    }
    return arity_9(const_cast<jit_closure *>(this),
                   a1.data,
                   a2.data,
                   a3.data,
                   a4.data,
                   a5.data,
                   a6.data,
                   a7.data,
                   a8.data,
                   a9.data);
  }

  object_ref jit_closure::call(object_ref const a1,
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
      throw invalid_arity<10>{ runtime::to_string(this) };
    }
    return arity_10(const_cast<jit_closure *>(this),
                    a1.data,
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

  callable_arity_flags jit_closure::get_arity_flags() const
  {
    return arity_flags;
  }
}
