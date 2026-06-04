#include <jank/runtime/obj/native_array_sequence.hpp>
#include <jank/runtime/obj/jit_variadic_closure.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/call.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  jit_variadic_closure::jit_variadic_closure()
    : object{ obj_type, obj_behaviors }
  {
  }

  jit_variadic_closure::jit_variadic_closure(callable_arity_flags const arity_flags,
                                             void * const context)
    : object{ obj_type, obj_behaviors }
    , context{ context }
    , arity_flags{ arity_flags }
  {
  }

  jit_variadic_closure::jit_variadic_closure(object_ref const meta)
    : object{ obj_type, obj_behaviors }
    , meta{ meta }
  {
  }

  void jit_variadic_closure::to_string(jtl::string_builder &buff) const
  {
    auto const name(meta.get(__rt_ctx->intern_keyword("name").expect_ok()));
    util::format_to(
      buff,
      "#object [{} {} {}]",
      (name.get_type() == object_type::nil ? "unknown" : try_object<persistent_string>(name)->data),
      object_type_str(type),
      this);
  }

  jit_variadic_closure_ref jit_variadic_closure::with_meta(object_ref const m)
  {
    auto const new_meta(behavior::detail::validate_meta(m));
    auto const ret{ make_box<jit_variadic_closure>(*this) };
    ret->meta = new_meta;
    return ret;
  }

  object_ref jit_variadic_closure::get_meta() const
  {
    return meta;
  }

  /* TODO: Share this, rather than dupe it. */
  template <typename F, typename... Args>
  object_ref do_call(F const fn, jit_variadic_closure const * const obj, Args... args)
  {
    if(!fn)
    {
      throw invalid_arity<sizeof...(Args)>{ obj->to_code_string() };
    }

    return fn(runtime::detail::untagged(obj), args...);
  }

  object_ref jit_variadic_closure::call() const
  {
    switch(arity_flags)
    {
      case mask_variadic_arity(0):
        return do_call(arity_1, this, object_ref{});
      default:
        return do_call(arity_0, this);
    }
    return do_call(arity_0, this);
  }

  object_ref jit_variadic_closure::call(object_ref const a1) const
  {
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return do_call(arity_1, this, make_box<obj::native_array_sequence>(a1));
      case mask_variadic_arity(1):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return do_call(arity_2, this, a1, object_ref{});
        }
      default:
        return do_call(arity_1, this, a1);
    }
  }

  object_ref jit_variadic_closure::call(object_ref const a1, object_ref const a2) const
  {
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return do_call(arity_1, this, make_box<obj::native_array_sequence>(a1, a2));
      case mask_variadic_arity(1):
        return do_call(arity_2, this, a1, make_box<obj::native_array_sequence>(a2));
      case mask_variadic_arity(2):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return do_call(arity_3, this, a1, a2, object_ref{});
        }
      default:
        return do_call(arity_2, this, a1, a2);
    }
  }

  object_ref
  jit_variadic_closure::call(object_ref const a1, object_ref const a2, object_ref const a3) const
  {
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return do_call(arity_1, this, make_box<obj::native_array_sequence>(a1, a2, a3));
      case mask_variadic_arity(1):
        return do_call(arity_2, this, a1, make_box<obj::native_array_sequence>(a2, a3));
      case mask_variadic_arity(2):
        return do_call(arity_3, this, a1, a2, make_box<obj::native_array_sequence>(a3));
      case mask_variadic_arity(3):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return do_call(arity_4, this, a1, a2, a3, object_ref{});
        }
      default:
        return do_call(arity_3, this, a1, a2, a3);
    }
  }

  object_ref jit_variadic_closure::call(object_ref const a1,
                                        object_ref const a2,
                                        object_ref const a3,
                                        object_ref const a4) const
  {
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return do_call(arity_1, this, make_box<obj::native_array_sequence>(a1, a2, a3, a4));
      case mask_variadic_arity(1):
        return do_call(arity_2, this, a1, make_box<obj::native_array_sequence>(a2, a3, a4));
      case mask_variadic_arity(2):
        return do_call(arity_3, this, a1, a2, make_box<obj::native_array_sequence>(a3, a4));
      case mask_variadic_arity(3):
        return do_call(arity_4, this, a1, a2, a3, make_box<obj::native_array_sequence>(a4));
      case mask_variadic_arity(4):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return do_call(arity_5, this, a1, a2, a3, a4, object_ref{});
        }
      default:
        return do_call(arity_4, this, a1, a2, a3, a4);
    }
  }

  object_ref jit_variadic_closure::call(object_ref const a1,
                                        object_ref const a2,
                                        object_ref const a3,
                                        object_ref const a4,
                                        object_ref const a5) const
  {
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return do_call(arity_1, this, make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5));
      case mask_variadic_arity(1):
        return do_call(arity_2, this, a1, make_box<obj::native_array_sequence>(a2, a3, a4, a5));
      case mask_variadic_arity(2):
        return do_call(arity_3, this, a1, a2, make_box<obj::native_array_sequence>(a3, a4, a5));
      case mask_variadic_arity(3):
        return do_call(arity_4, this, a1, a2, a3, make_box<obj::native_array_sequence>(a4, a5));
      case mask_variadic_arity(4):
        return do_call(arity_5, this, a1, a2, a3, a4, make_box<obj::native_array_sequence>(a5));
      case mask_variadic_arity(5):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return do_call(arity_6, this, a1, a2, a3, a4, a5, object_ref{});
        }
      default:
        return do_call(arity_5, this, a1, a2, a3, a4, a5);
    }
  }

  object_ref jit_variadic_closure::call(object_ref const a1,
                                        object_ref const a2,
                                        object_ref const a3,
                                        object_ref const a4,
                                        object_ref const a5,
                                        object_ref const a6) const
  {
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return do_call(arity_1, this, make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6));
      case mask_variadic_arity(1):
        return do_call(arity_2, this, a1, make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6));
      case mask_variadic_arity(2):
        return do_call(arity_3, this, a1, a2, make_box<obj::native_array_sequence>(a3, a4, a5, a6));
      case mask_variadic_arity(3):
        return do_call(arity_4, this, a1, a2, a3, make_box<obj::native_array_sequence>(a4, a5, a6));
      case mask_variadic_arity(4):
        return do_call(arity_5, this, a1, a2, a3, a4, make_box<obj::native_array_sequence>(a5, a6));
      case mask_variadic_arity(5):
        return do_call(arity_6, this, a1, a2, a3, a4, a5, make_box<obj::native_array_sequence>(a6));
      case mask_variadic_arity(6):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return do_call(arity_7, this, a1, a2, a3, a4, a5, a6, object_ref{});
        }
      default:
        return do_call(arity_6, this, a1, a2, a3, a4, a5, a6);
    }
  }

  object_ref jit_variadic_closure::call(object_ref const a1,
                                        object_ref const a2,
                                        object_ref const a3,
                                        object_ref const a4,
                                        object_ref const a5,
                                        object_ref const a6,
                                        object_ref const a7) const
  {
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return do_call(arity_1,
                       this,
                       make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7));
      case mask_variadic_arity(1):
        return do_call(arity_2,
                       this,
                       a1,
                       make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7));
      case mask_variadic_arity(2):
        return do_call(arity_3,
                       this,
                       a1,
                       a2,
                       make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7));
      case mask_variadic_arity(3):
        return do_call(arity_4,
                       this,
                       a1,
                       a2,
                       a3,
                       make_box<obj::native_array_sequence>(a4, a5, a6, a7));
      case mask_variadic_arity(4):
        return do_call(arity_5,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       make_box<obj::native_array_sequence>(a5, a6, a7));
      case mask_variadic_arity(5):
        return do_call(arity_6,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       make_box<obj::native_array_sequence>(a6, a7));
      case mask_variadic_arity(6):
        return do_call(arity_7,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       make_box<obj::native_array_sequence>(a7));
      case mask_variadic_arity(7):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return do_call(arity_8, this, a1, a2, a3, a4, a5, a6, a7, object_ref{});
        }
      default:
        return do_call(arity_7, this, a1, a2, a3, a4, a5, a6, a7);
    }
  }

  object_ref jit_variadic_closure::call(object_ref const a1,
                                        object_ref const a2,
                                        object_ref const a3,
                                        object_ref const a4,
                                        object_ref const a5,
                                        object_ref const a6,
                                        object_ref const a7,
                                        object_ref const a8) const
  {
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return do_call(arity_1,
                       this,
                       make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7, a8));
      case mask_variadic_arity(1):
        return do_call(arity_2,
                       this,
                       a1,
                       make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7, a8));
      case mask_variadic_arity(2):
        return do_call(arity_3,
                       this,
                       a1,
                       a2,
                       make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7, a8));
      case mask_variadic_arity(3):
        return do_call(arity_4,
                       this,
                       a1,
                       a2,
                       a3,
                       make_box<obj::native_array_sequence>(a4, a5, a6, a7, a8));
      case mask_variadic_arity(4):
        return do_call(arity_5,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       make_box<obj::native_array_sequence>(a5, a6, a7, a8));
      case mask_variadic_arity(5):
        return do_call(arity_6,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       make_box<obj::native_array_sequence>(a6, a7, a8));
      case mask_variadic_arity(6):
        return do_call(arity_7,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       make_box<obj::native_array_sequence>(a7, a8));
      case mask_variadic_arity(7):
        return do_call(arity_8,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       a7,
                       make_box<obj::native_array_sequence>(a8));
      case mask_variadic_arity(8):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return do_call(arity_9, this, a1, a2, a3, a4, a5, a6, a7, a8, object_ref{});
        }
      default:
        return do_call(arity_8, this, a1, a2, a3, a4, a5, a6, a7, a8);
    }
  }

  object_ref jit_variadic_closure::call(object_ref const a1,
                                        object_ref const a2,
                                        object_ref const a3,
                                        object_ref const a4,
                                        object_ref const a5,
                                        object_ref const a6,
                                        object_ref const a7,
                                        object_ref const a8,
                                        object_ref const a9) const
  {
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return do_call(arity_1,
                       this,
                       make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7, a8, a9));
      case mask_variadic_arity(1):
        return do_call(arity_2,
                       this,
                       a1,
                       make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7, a8, a9));
      case mask_variadic_arity(2):
        return do_call(arity_3,
                       this,
                       a1,
                       a2,
                       make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7, a8, a9));
      case mask_variadic_arity(3):
        return do_call(arity_4,
                       this,
                       a1,
                       a2,
                       a3,
                       make_box<obj::native_array_sequence>(a4, a5, a6, a7, a8, a9));
      case mask_variadic_arity(4):
        return do_call(arity_5,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       make_box<obj::native_array_sequence>(a5, a6, a7, a8, a9));
      case mask_variadic_arity(5):
        return do_call(arity_6,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       make_box<obj::native_array_sequence>(a6, a7, a8, a9));
      case mask_variadic_arity(6):
        return do_call(arity_7,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       make_box<obj::native_array_sequence>(a7, a8, a9));
      case mask_variadic_arity(7):
        return do_call(arity_8,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       a7,
                       make_box<obj::native_array_sequence>(a8, a9));
      case mask_variadic_arity(8):
        return do_call(arity_9,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       a7,
                       a8,
                       make_box<obj::native_array_sequence>(a9));
      case mask_variadic_arity(9):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return do_call(arity_10, this, a1, a2, a3, a4, a5, a6, a7, a8, a9, object_ref{});
        }
      default:
        return do_call(arity_9, this, a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
  }

  object_ref jit_variadic_closure::call(object_ref const a1,
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
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return do_call(
          arity_1,
          this,
          make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10));
      case mask_variadic_arity(1):
        return do_call(arity_2,
                       this,
                       a1,
                       make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7, a8, a9, a10));
      case mask_variadic_arity(2):
        return do_call(arity_3,
                       this,
                       a1,
                       a2,
                       make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7, a8, a9, a10));
      case mask_variadic_arity(3):
        return do_call(arity_4,
                       this,
                       a1,
                       a2,
                       a3,
                       make_box<obj::native_array_sequence>(a4, a5, a6, a7, a8, a9, a10));
      case mask_variadic_arity(4):
        return do_call(arity_5,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       make_box<obj::native_array_sequence>(a5, a6, a7, a8, a9, a10));
      case mask_variadic_arity(5):
        return do_call(arity_6,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       make_box<obj::native_array_sequence>(a6, a7, a8, a9, a10));
      case mask_variadic_arity(6):
        return do_call(arity_7,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       make_box<obj::native_array_sequence>(a7, a8, a9, a10));
      case mask_variadic_arity(7):
        return do_call(arity_8,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       a7,
                       make_box<obj::native_array_sequence>(a8, a9, a10));
      case mask_variadic_arity(8):
        return do_call(arity_9,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       a7,
                       a8,
                       make_box<obj::native_array_sequence>(a9, a10));
      case mask_variadic_arity(9):
        return do_call(arity_10,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       a7,
                       a8,
                       a9,
                       make_box<obj::native_array_sequence>(a10));
      case mask_variadic_arity(10):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return do_call(arity_10,
                         this,
                         a1,
                         a2,
                         a3,
                         a4,
                         a5,
                         a6,
                         a7,
                         a8,
                         a9,
                         make_box<obj::native_array_sequence>(a10));
        }
      default:
        return do_call(arity_10, this, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    }
  }

  object_ref jit_variadic_closure::call(object_ref const a1,
                                        object_ref const a2,
                                        object_ref const a3,
                                        object_ref const a4,
                                        object_ref const a5,
                                        object_ref const a6,
                                        object_ref const a7,
                                        object_ref const a8,
                                        object_ref const a9,
                                        object_ref const a10,
                                        object_ref const more) const
  {
    auto const mask(extract_variadic_arity_mask(arity_flags));

    static auto const concat{ __rt_ctx->intern_var("clojure.core/concat*").expect_ok()->deref() };

    switch(mask)
    {
      case mask_variadic_arity(0):
        return do_call(
          arity_1,
          this,
          concat.call(make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10),
                      more));
      case mask_variadic_arity(1):
        return do_call(
          arity_2,
          this,
          a1,
          concat.call(make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7, a8, a9, a10),
                      more));
      case mask_variadic_arity(2):
        return do_call(
          arity_3,
          this,
          a1,
          a2,
          concat.call(make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7, a8, a9, a10), more));
      case mask_variadic_arity(3):
        return do_call(
          arity_4,
          this,
          a1,
          a2,
          a3,
          concat.call(make_box<obj::native_array_sequence>(a4, a5, a6, a7, a8, a9, a10), more));
      case mask_variadic_arity(4):
        return do_call(
          arity_5,
          this,
          a1,
          a2,
          a3,
          a4,
          concat.call(make_box<obj::native_array_sequence>(a5, a6, a7, a8, a9, a10), more));
      case mask_variadic_arity(5):
        return do_call(
          arity_6,
          this,
          a1,
          a2,
          a3,
          a4,
          a5,
          concat.call(make_box<obj::native_array_sequence>(a6, a7, a8, a9, a10), more));
      case mask_variadic_arity(6):
        return do_call(arity_7,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       concat.call(make_box<obj::native_array_sequence>(a7, a8, a9, a10), more));
      case mask_variadic_arity(7):
        return do_call(arity_8,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       a7,
                       concat.call(make_box<obj::native_array_sequence>(a8, a9, a10), more));
      case mask_variadic_arity(8):
        return do_call(arity_9,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       a7,
                       a8,
                       concat.call(make_box<obj::native_array_sequence>(a9, a10), more));
      case mask_variadic_arity(9):
        return do_call(arity_10,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       a7,
                       a8,
                       a9,
                       concat.call(make_box<obj::native_array_sequence>(a10), more));
      case mask_variadic_arity(10):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return do_call(arity_10,
                         this,
                         a1,
                         a2,
                         a3,
                         a4,
                         a5,
                         a6,
                         a7,
                         a8,
                         a9,
                         concat.call(make_box<obj::native_array_sequence>(a10), more));
        }
      default:
        return do_call(arity_10,
                       this,
                       a1,
                       a2,
                       a3,
                       a4,
                       a5,
                       a6,
                       a7,
                       a8,
                       a9,
                       concat.call(make_box<obj::native_array_sequence>(a10), more));
    }
  }
}
