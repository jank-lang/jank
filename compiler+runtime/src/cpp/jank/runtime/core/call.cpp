#include <jank/runtime/core/call.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/util/make_array.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime
{
  using namespace behavior;

  static object_ref pass_through_vars(object_ref source)
  {
    while(source->type == object_type::var)
    {
      source = runtime::deref(source);
    }
    return source;
  }

  object_ref dynamic_call(object_ref const source)
  {
    auto const processed_source(pass_through_vars(source));
    auto const arity_flags(processed_source->get_arity_flags());

    switch(arity_flags)
    {
      case mask_variadic_arity(0):
        return processed_source->call(jank_nil());
      default:
        return processed_source->call();
    }
  }

  object_ref dynamic_call(object_ref const source, object_ref const a1)
  {
    auto const processed_source(pass_through_vars(source));
    auto const arity_flags(processed_source->get_arity_flags());
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return processed_source->call(make_box<obj::native_array_sequence>(a1));
      case mask_variadic_arity(1):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return processed_source->call(a1, jank_nil());
        }
      default:
        return processed_source->call(a1);
    }
  }

  object_ref dynamic_call(object_ref const source, object_ref const a1, object_ref const a2)
  {
    auto const processed_source(pass_through_vars(source));
    auto const arity_flags(processed_source->get_arity_flags());
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return processed_source->call(make_box<obj::native_array_sequence>(a1, a2));
      case mask_variadic_arity(1):
        return processed_source->call(a1, make_box<obj::native_array_sequence>(a2));
      case mask_variadic_arity(2):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return processed_source->call(a1, a2, jank_nil());
        }
      default:
        return processed_source->call(a1, a2);
    }
  }

  object_ref dynamic_call(object_ref const source,
                          object_ref const a1,
                          object_ref const a2,
                          object_ref const a3)
  {
    auto const processed_source(pass_through_vars(source));
    auto const arity_flags(processed_source->get_arity_flags());
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return processed_source->call(make_box<obj::native_array_sequence>(a1, a2, a3));
      case mask_variadic_arity(1):
        return processed_source->call(a1, make_box<obj::native_array_sequence>(a2, a3));
      case mask_variadic_arity(2):
        return processed_source->call(a1, a2, make_box<obj::native_array_sequence>(a3));
      case mask_variadic_arity(3):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return processed_source->call(a1, a2, a3, jank_nil());
        }
      default:
        return processed_source->call(a1, a2, a3);
    }
  }

  object_ref dynamic_call(object_ref const source,
                          object_ref const a1,
                          object_ref const a2,
                          object_ref const a3,
                          object_ref const a4)
  {
    auto const processed_source(pass_through_vars(source));
    auto const arity_flags(processed_source->get_arity_flags());
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return processed_source->call(make_box<obj::native_array_sequence>(a1, a2, a3, a4));
      case mask_variadic_arity(1):
        return processed_source->call(a1, make_box<obj::native_array_sequence>(a2, a3, a4));
      case mask_variadic_arity(2):
        return processed_source->call(a1, a2, make_box<obj::native_array_sequence>(a3, a4));
      case mask_variadic_arity(3):
        return processed_source->call(a1, a2, a3, make_box<obj::native_array_sequence>(a4));
      case mask_variadic_arity(4):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return processed_source->call(a1, a2, a3, a4, jank_nil());
        }
      default:
        return processed_source->call(a1, a2, a3, a4);
    }
  }

  object_ref dynamic_call(object_ref const source,
                          object_ref const a1,
                          object_ref const a2,
                          object_ref const a3,
                          object_ref const a4,
                          object_ref const a5)
  {
    auto const processed_source(pass_through_vars(source));
    auto const arity_flags(processed_source->get_arity_flags());
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return processed_source->call(make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5));
      case mask_variadic_arity(1):
        return processed_source->call(a1, make_box<obj::native_array_sequence>(a2, a3, a4, a5));
      case mask_variadic_arity(2):
        return processed_source->call(a1, a2, make_box<obj::native_array_sequence>(a3, a4, a5));
      case mask_variadic_arity(3):
        return processed_source->call(a1, a2, a3, make_box<obj::native_array_sequence>(a4, a5));
      case mask_variadic_arity(4):
        return processed_source->call(a1, a2, a3, a4, make_box<obj::native_array_sequence>(a5));
      case mask_variadic_arity(5):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return processed_source->call(a1, a2, a3, a4, a5, jank_nil());
        }
      default:
        return processed_source->call(a1, a2, a3, a4, a5);
    }
  }

  object_ref dynamic_call(object_ref const source,
                          object_ref const a1,
                          object_ref const a2,
                          object_ref const a3,
                          object_ref const a4,
                          object_ref const a5,
                          object_ref const a6)
  {
    auto const processed_source(pass_through_vars(source));
    auto const arity_flags(processed_source->get_arity_flags());
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return processed_source->call(make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6));
      case mask_variadic_arity(1):
        return processed_source->call(a1, make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6));
      case mask_variadic_arity(2):
        return processed_source->call(a1, a2, make_box<obj::native_array_sequence>(a3, a4, a5, a6));
      case mask_variadic_arity(3):
        return processed_source->call(a1, a2, a3, make_box<obj::native_array_sequence>(a4, a5, a6));
      case mask_variadic_arity(4):
        return processed_source->call(a1, a2, a3, a4, make_box<obj::native_array_sequence>(a5, a6));
      case mask_variadic_arity(5):
        return processed_source->call(a1, a2, a3, a4, a5, make_box<obj::native_array_sequence>(a6));
      case mask_variadic_arity(6):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return processed_source->call(a1, a2, a3, a4, a5, a6, jank_nil());
        }
      default:
        return processed_source->call(a1, a2, a3, a4, a5, a6);
    }
  }

  object_ref dynamic_call(object_ref const source,
                          object_ref const a1,
                          object_ref const a2,
                          object_ref const a3,
                          object_ref const a4,
                          object_ref const a5,
                          object_ref const a6,
                          object_ref const a7)
  {
    auto const processed_source(pass_through_vars(source));
    auto const arity_flags(processed_source->get_arity_flags());
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return processed_source->call(
          make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7));
      case mask_variadic_arity(1):
        return processed_source->call(a1,
                                      make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7));
      case mask_variadic_arity(2):
        return processed_source->call(a1,
                                      a2,
                                      make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7));
      case mask_variadic_arity(3):
        return processed_source->call(a1,
                                      a2,
                                      a3,
                                      make_box<obj::native_array_sequence>(a4, a5, a6, a7));
      case mask_variadic_arity(4):
        return processed_source->call(a1,
                                      a2,
                                      a3,
                                      a4,
                                      make_box<obj::native_array_sequence>(a5, a6, a7));
      case mask_variadic_arity(5):
        return processed_source
          ->call(a1, a2, a3, a4, a5, make_box<obj::native_array_sequence>(a6, a7));
      case mask_variadic_arity(6):
        return processed_source
          ->call(a1, a2, a3, a4, a5, a6, make_box<obj::native_array_sequence>(a7));
      case mask_variadic_arity(7):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return processed_source->call(a1, a2, a3, a4, a5, a6, a7, jank_nil());
        }
      default:
        return processed_source->call(a1, a2, a3, a4, a5, a6, a7);
    }
  }

  object_ref dynamic_call(object_ref const source,
                          object_ref const a1,
                          object_ref const a2,
                          object_ref const a3,
                          object_ref const a4,
                          object_ref const a5,
                          object_ref const a6,
                          object_ref const a7,
                          object_ref const a8)
  {
    auto const processed_source(pass_through_vars(source));
    auto const arity_flags(processed_source->get_arity_flags());
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return processed_source->call(
          make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7, a8));
      case mask_variadic_arity(1):
        return processed_source->call(
          a1,
          make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7, a8));
      case mask_variadic_arity(2):
        return processed_source->call(a1,
                                      a2,
                                      make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7, a8));
      case mask_variadic_arity(3):
        return processed_source->call(a1,
                                      a2,
                                      a3,
                                      make_box<obj::native_array_sequence>(a4, a5, a6, a7, a8));
      case mask_variadic_arity(4):
        return processed_source->call(a1,
                                      a2,
                                      a3,
                                      a4,
                                      make_box<obj::native_array_sequence>(a5, a6, a7, a8));
      case mask_variadic_arity(5):
        return processed_source
          ->call(a1, a2, a3, a4, a5, make_box<obj::native_array_sequence>(a6, a7, a8));
      case mask_variadic_arity(6):
        return processed_source
          ->call(a1, a2, a3, a4, a5, a6, make_box<obj::native_array_sequence>(a7, a8));
      case mask_variadic_arity(7):
        return processed_source
          ->call(a1, a2, a3, a4, a5, a6, a7, make_box<obj::native_array_sequence>(a8));
      case mask_variadic_arity(8):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return processed_source->call(a1, a2, a3, a4, a5, a6, a7, a8, jank_nil());
        }
      default:
        return processed_source->call(a1, a2, a3, a4, a5, a6, a7, a8);
    }
  }

  object_ref dynamic_call(object_ref const source,
                          object_ref const a1,
                          object_ref const a2,
                          object_ref const a3,
                          object_ref const a4,
                          object_ref const a5,
                          object_ref const a6,
                          object_ref const a7,
                          object_ref const a8,
                          object_ref const a9)
  {
    auto const processed_source(pass_through_vars(source));
    auto const arity_flags(processed_source->get_arity_flags());
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return processed_source->call(
          make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7, a8, a9));
      case mask_variadic_arity(1):
        return processed_source->call(
          a1,
          make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7, a8, a9));
      case mask_variadic_arity(2):
        return processed_source->call(
          a1,
          a2,
          make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7, a8, a9));
      case mask_variadic_arity(3):
        return processed_source->call(a1,
                                      a2,
                                      a3,
                                      make_box<obj::native_array_sequence>(a4, a5, a6, a7, a8, a9));
      case mask_variadic_arity(4):
        return processed_source->call(a1,
                                      a2,
                                      a3,
                                      a4,
                                      make_box<obj::native_array_sequence>(a5, a6, a7, a8, a9));
      case mask_variadic_arity(5):
        return processed_source
          ->call(a1, a2, a3, a4, a5, make_box<obj::native_array_sequence>(a6, a7, a8, a9));
      case mask_variadic_arity(6):
        return processed_source
          ->call(a1, a2, a3, a4, a5, a6, make_box<obj::native_array_sequence>(a7, a8, a9));
      case mask_variadic_arity(7):
        return processed_source
          ->call(a1, a2, a3, a4, a5, a6, a7, make_box<obj::native_array_sequence>(a8, a9));
      case mask_variadic_arity(8):
        return processed_source
          ->call(a1, a2, a3, a4, a5, a6, a7, a8, make_box<obj::native_array_sequence>(a9));
      case mask_variadic_arity(9):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return processed_source->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, jank_nil());
        }
      default:
        return processed_source->call(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
  }

  object_ref dynamic_call(object_ref const source,
                          object_ref const a1,
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
    auto const processed_source(pass_through_vars(source));
    auto const arity_flags(processed_source->get_arity_flags());
    auto const mask(extract_variadic_arity_mask(arity_flags));

    switch(mask)
    {
      case mask_variadic_arity(0):
        return processed_source->call(
          make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10));
      case mask_variadic_arity(1):
        return processed_source->call(
          a1,
          make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7, a8, a9, a10));
      case mask_variadic_arity(2):
        return processed_source->call(
          a1,
          a2,
          make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7, a8, a9, a10));
      case mask_variadic_arity(3):
        return processed_source
          ->call(a1, a2, a3, make_box<obj::native_array_sequence>(a4, a5, a6, a7, a8, a9, a10));
      case mask_variadic_arity(4):
        return processed_source
          ->call(a1, a2, a3, a4, make_box<obj::native_array_sequence>(a5, a6, a7, a8, a9, a10));
      case mask_variadic_arity(5):
        return processed_source
          ->call(a1, a2, a3, a4, a5, make_box<obj::native_array_sequence>(a6, a7, a8, a9, a10));
      case mask_variadic_arity(6):
        return processed_source
          ->call(a1, a2, a3, a4, a5, a6, make_box<obj::native_array_sequence>(a7, a8, a9, a10));
      case mask_variadic_arity(7):
        return processed_source
          ->call(a1, a2, a3, a4, a5, a6, a7, make_box<obj::native_array_sequence>(a8, a9, a10));
      case mask_variadic_arity(8):
        return processed_source
          ->call(a1, a2, a3, a4, a5, a6, a7, a8, make_box<obj::native_array_sequence>(a9, a10));
      case mask_variadic_arity(9):
        return processed_source
          ->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, make_box<obj::native_array_sequence>(a10));
      case mask_variadic_arity(10):
        if(!is_variadic_ambiguous(arity_flags))
        {
          return processed_source
            ->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, make_box<obj::native_array_sequence>(a10));
        }
      default:
        return processed_source->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    }
  }

  object_ref dynamic_call(object_ref const source,
                          object_ref const a1,
                          object_ref const a2,
                          object_ref const a3,
                          object_ref const a4,
                          object_ref const a5,
                          object_ref const a6,
                          object_ref const a7,
                          object_ref const a8,
                          object_ref const a9,
                          object_ref const a10,
                          obj::persistent_list_ref const rest)
  {
    /* TODO: Move call fns into var so we can remove these checks. */
    auto const processed_source(pass_through_vars(source));
    auto const arity_flags(processed_source->get_arity_flags());
    auto const mask(extract_variadic_arity_mask(arity_flags));
    switch(mask)
    {
      /* TODO: Optimize this with a faster seq? */
      case mask_variadic_arity(0):
        {
          native_vector<object_ref> packed;
          packed.reserve(10 + rest->count());
          packed.insert(packed.end(), { a1, a2, a3, a4, a5, a6, a7, a8, a9, a10 });
          std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
          return processed_source->call(make_box<obj::native_vector_sequence>(std::move(packed)));
        }
      case mask_variadic_arity(1):
        {
          native_vector<object_ref> packed;
          packed.reserve(9 + rest->count());
          packed.insert(packed.end(), { a2, a3, a4, a5, a6, a7, a8, a9, a10 });
          std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
          return processed_source->call(a1,
                                        make_box<obj::native_vector_sequence>(std::move(packed)));
        }
      case mask_variadic_arity(2):
        {
          native_vector<object_ref> packed;
          packed.reserve(8 + rest->count());
          packed.insert(packed.end(), { a3, a4, a5, a6, a7, a8, a9, a10 });
          std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
          return processed_source->call(a1,
                                        a2,
                                        make_box<obj::native_vector_sequence>(std::move(packed)));
        }
      case mask_variadic_arity(3):
        {
          native_vector<object_ref> packed;
          packed.reserve(7 + rest->count());
          packed.insert(packed.end(), { a4, a5, a6, a7, a8, a9, a10 });
          std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
          return processed_source->call(a1,
                                        a2,
                                        a3,
                                        make_box<obj::native_vector_sequence>(std::move(packed)));
        }
      case mask_variadic_arity(4):
        {
          native_vector<object_ref> packed;
          packed.reserve(6 + rest->count());
          packed.insert(packed.end(), { a5, a6, a7, a8, a9, a10 });
          std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
          return processed_source->call(a1,
                                        a2,
                                        a3,
                                        a4,
                                        make_box<obj::native_vector_sequence>(std::move(packed)));
        }
      case mask_variadic_arity(5):
        {
          native_vector<object_ref> packed;
          packed.reserve(5 + rest->count());
          packed.insert(packed.end(), { a6, a7, a8, a9, a10 });
          std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
          return processed_source
            ->call(a1, a2, a3, a4, a5, make_box<obj::native_vector_sequence>(std::move(packed)));
        }
      case mask_variadic_arity(6):
        {
          native_vector<object_ref> packed;
          packed.reserve(4 + rest->count());
          packed.insert(packed.end(), { a7, a8, a9, a10 });
          std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
          return processed_source->call(a1,
                                        a2,
                                        a3,
                                        a4,
                                        a5,
                                        a6,
                                        make_box<obj::native_vector_sequence>(std::move(packed)));
        }
      case mask_variadic_arity(7):
        {
          native_vector<object_ref> packed;
          packed.reserve(3 + rest->count());
          packed.insert(packed.end(), { a8, a9, a10 });
          std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
          return processed_source->call(a1,
                                        a2,
                                        a3,
                                        a4,
                                        a5,
                                        a6,
                                        a7,
                                        make_box<obj::native_vector_sequence>(std::move(packed)));
        }
      case mask_variadic_arity(8):
        {
          native_vector<object_ref> packed;
          packed.reserve(2 + rest->count());
          packed.insert(packed.end(), { a9, a10 });
          std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
          return processed_source->call(a1,
                                        a2,
                                        a3,
                                        a4,
                                        a5,
                                        a6,
                                        a7,
                                        a8,
                                        make_box<obj::native_vector_sequence>(std::move(packed)));
        }
      case mask_variadic_arity(9):
        {
          native_vector<object_ref> packed;
          packed.reserve(1 + rest->count());
          packed.insert(packed.end(), { a10 });
          std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
          return processed_source->call(a1,
                                        a2,
                                        a3,
                                        a4,
                                        a5,
                                        a6,
                                        a7,
                                        a8,
                                        a9,
                                        make_box<obj::native_vector_sequence>(std::move(packed)));
        }
      default:
        throw std::runtime_error{ util::format("unsupported arity: {}", 10 + rest->count()) };
    }
  }

  object_ref apply_to(object_ref const source, object_ref const args)
  {
    return visit_seqable(
      [=](auto const typed_args) -> object_ref {
        auto const s(typed_args->fresh_seq());
        auto const length(sequence_length(s, max_params + 1));
        auto const r{ make_sequence_range(s) };
        auto it{ r.begin() };

        /* XXX: It's UB to have multiple *(++it) in one expression, since the order in
         * which they will run is not defined. This requires us to pull out all of the
         * args into locals. */
        switch(length)
        {
          case 0:
            return dynamic_call(source);
          case 1:
            {
              auto const a1{ *it };
              return dynamic_call(source, a1);
            }
          case 2:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              return dynamic_call(source, a1, a2);
            }
          case 3:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              return dynamic_call(source, a1, a2, a3);
            }
          case 4:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              return dynamic_call(source, a1, a2, a3, a4);
            }
          case 5:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              auto const a5{ *(++it) };
              return dynamic_call(source, a1, a2, a3, a4, a5);
            }
          case 6:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              auto const a5{ *(++it) };
              auto const a6{ *(++it) };
              return dynamic_call(source, a1, a2, a3, a4, a5, a6);
            }
          case 7:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              auto const a5{ *(++it) };
              auto const a6{ *(++it) };
              auto const a7{ *(++it) };
              return dynamic_call(source, a1, a2, a3, a4, a5, a6, a7);
            }
          case 8:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              auto const a5{ *(++it) };
              auto const a6{ *(++it) };
              auto const a7{ *(++it) };
              auto const a8{ *(++it) };
              return dynamic_call(source, a1, a2, a3, a4, a5, a6, a7, a8);
            }
          case 9:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              auto const a5{ *(++it) };
              auto const a6{ *(++it) };
              auto const a7{ *(++it) };
              auto const a8{ *(++it) };
              auto const a9{ *(++it) };
              return dynamic_call(source, a1, a2, a3, a4, a5, a6, a7, a8, a9);
            }
          case 10:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              auto const a5{ *(++it) };
              auto const a6{ *(++it) };
              auto const a7{ *(++it) };
              auto const a8{ *(++it) };
              auto const a9{ *(++it) };
              auto const a10{ *(++it) };
              return dynamic_call(source, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
            }
          default:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              auto const a5{ *(++it) };
              auto const a6{ *(++it) };
              auto const a7{ *(++it) };
              auto const a8{ *(++it) };
              auto const a9{ *(++it) };
              auto const a10{ *(++it) };
              return dynamic_call(source,
                                  a1,
                                  a2,
                                  a3,
                                  a4,
                                  a5,
                                  a6,
                                  a7,
                                  a8,
                                  a9,
                                  a10,
                                  obj::persistent_list::create((++it).data));
            }
        }
      },
      args);
  }
}
