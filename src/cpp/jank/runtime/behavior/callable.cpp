#include <fmt/core.h>

#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/native_array_sequence.hpp>
#include <jank/runtime/obj/native_vector_sequence.hpp>
#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/seq.hpp>
#include <jank/util/make_array.hpp>

namespace jank::runtime
{
  using namespace behavior;

  object_ptr dynamic_call(object_ptr const source)
  {
    return visit_object(
      [=](auto const typed_source) -> object_ptr {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(function_like<T> || std::is_base_of_v<callable, T>)
        {
          auto const arity_flags(typed_source->get_arity_flags());

          switch(arity_flags)
          {
            case callable::mask_variadic_arity(0):
              return typed_source->call(obj::nil::nil_const());
            default:
              return typed_source->call();
          }
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid call with 0 args to {}",
                                                typed_source->to_string()) };
        }
      },
      source);
  }

  object_ptr dynamic_call(object_ptr const source, object_ptr const a1)
  {
    return visit_object(
      [=](auto const typed_source) -> object_ptr {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(function_like<T> || std::is_base_of_v<callable, T>)
        {
          auto const arity_flags(typed_source->get_arity_flags());
          auto const mask(callable::extract_variadic_arity_mask(arity_flags));

          switch(mask)
          {
            case callable::mask_variadic_arity(0):
              return typed_source->call(make_box<obj::native_array_sequence>(a1));
            case callable::mask_variadic_arity(1):
              if(!callable::is_variadic_ambiguous(arity_flags))
              {
                return typed_source->call(a1, obj::nil::nil_const());
              }
            default:
              return typed_source->call(a1);
          }
        }
        else if constexpr(std::same_as<T, obj::persistent_set>
                          || std::same_as<T, obj::persistent_hash_map>
                          || std::same_as<T, obj::persistent_array_map>
                          || std::same_as<T, obj::transient_vector>
                          || std::same_as<T, obj::transient_set> || std::same_as<T, obj::keyword>)
        {
          return typed_source->call(a1);
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid call with 1 arg to: {}",
                                                typed_source->to_string()) };
        }
      },
      source);
  }

  object_ptr dynamic_call(object_ptr const source, object_ptr const a1, object_ptr const a2)
  {
    return visit_object(
      [=](auto const typed_source) -> object_ptr {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(function_like<T> || std::is_base_of_v<callable, T>)
        {
          auto const arity_flags(typed_source->get_arity_flags());
          auto const mask(callable::extract_variadic_arity_mask(arity_flags));

          switch(mask)
          {
            case callable::mask_variadic_arity(0):
              return typed_source->call(make_box<obj::native_array_sequence>(a1, a2));
            case callable::mask_variadic_arity(1):
              return typed_source->call(a1, make_box<obj::native_array_sequence>(a2));
            case callable::mask_variadic_arity(2):
              if(!callable::is_variadic_ambiguous(arity_flags))
              {
                return typed_source->call(a1, a2, obj::nil::nil_const());
              }
            default:
              return typed_source->call(a1, a2);
          }
        }
        else if constexpr(std::same_as<T, obj::persistent_hash_map>
                          || std::same_as<T, obj::persistent_array_map>
                          || std::same_as<T, obj::transient_set> || std::same_as<T, obj::keyword>)
        {
          return typed_source->call(a1, a2);
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid call with 2 args to: {}",
                                                typed_source->to_string()) };
        }
      },
      source);
  }

  object_ptr dynamic_call(object_ptr const source,
                          object_ptr const a1,
                          object_ptr const a2,
                          object_ptr const a3)
  {
    return visit_object(
      [=](auto const typed_source) -> object_ptr {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(function_like<T> || std::is_base_of_v<callable, T>)
        {
          auto const arity_flags(typed_source->get_arity_flags());
          auto const mask(callable::extract_variadic_arity_mask(arity_flags));

          switch(mask)
          {
            case callable::mask_variadic_arity(0):
              return typed_source->call(make_box<obj::native_array_sequence>(a1, a2, a3));
            case callable::mask_variadic_arity(1):
              return typed_source->call(a1, make_box<obj::native_array_sequence>(a2, a3));
            case callable::mask_variadic_arity(2):
              return typed_source->call(a1, a2, make_box<obj::native_array_sequence>(a3));
            case callable::mask_variadic_arity(3):
              if(!callable::is_variadic_ambiguous(arity_flags))
              {
                return typed_source->call(a1, a2, a3, obj::nil::nil_const());
              }
            default:
              return typed_source->call(a1, a2, a3);
          }
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid call with 3 args to: {}",
                                                typed_source->to_string()) };
        }
      },
      source);
  }

  object_ptr dynamic_call(object_ptr const source,
                          object_ptr const a1,
                          object_ptr const a2,
                          object_ptr const a3,
                          object_ptr const a4)
  {
    return visit_object(
      [=](auto const typed_source) -> object_ptr {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(function_like<T> || std::is_base_of_v<callable, T>)
        {
          auto const arity_flags(typed_source->get_arity_flags());
          auto const mask(callable::extract_variadic_arity_mask(arity_flags));

          switch(mask)
          {
            case callable::mask_variadic_arity(0):
              return typed_source->call(make_box<obj::native_array_sequence>(a1, a2, a3, a4));
            case callable::mask_variadic_arity(1):
              return typed_source->call(a1, make_box<obj::native_array_sequence>(a2, a3, a4));
            case callable::mask_variadic_arity(2):
              return typed_source->call(a1, a2, make_box<obj::native_array_sequence>(a3, a4));
            case callable::mask_variadic_arity(3):
              return typed_source->call(a1, a2, a3, make_box<obj::native_array_sequence>(a4));
            case callable::mask_variadic_arity(4):
              if(!callable::is_variadic_ambiguous(arity_flags))
              {
                return typed_source->call(a1, a2, a3, a4, obj::nil::nil_const());
              }
            default:
              return typed_source->call(a1, a2, a3, a4);
          }
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid call with 4 args to: {}",
                                                typed_source->to_string()) };
        }
      },
      source);
  }

  object_ptr dynamic_call(object_ptr const source,
                          object_ptr const a1,
                          object_ptr const a2,
                          object_ptr const a3,
                          object_ptr const a4,
                          object_ptr const a5)
  {
    return visit_object(
      [=](auto const typed_source) -> object_ptr {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(function_like<T> || std::is_base_of_v<callable, T>)
        {
          auto const arity_flags(typed_source->get_arity_flags());
          auto const mask(callable::extract_variadic_arity_mask(arity_flags));

          switch(mask)
          {
            case callable::mask_variadic_arity(0):
              return typed_source->call(make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5));
            case callable::mask_variadic_arity(1):
              return typed_source->call(a1, make_box<obj::native_array_sequence>(a2, a3, a4, a5));
            case callable::mask_variadic_arity(2):
              return typed_source->call(a1, a2, make_box<obj::native_array_sequence>(a3, a4, a5));
            case callable::mask_variadic_arity(3):
              return typed_source->call(a1, a2, a3, make_box<obj::native_array_sequence>(a4, a5));
            case callable::mask_variadic_arity(4):
              return typed_source->call(a1, a2, a3, a4, make_box<obj::native_array_sequence>(a5));
            case callable::mask_variadic_arity(5):
              if(!callable::is_variadic_ambiguous(arity_flags))
              {
                return typed_source->call(a1, a2, a3, a4, a5, obj::nil::nil_const());
              }
            default:
              return typed_source->call(a1, a2, a3, a4, a5);
          }
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid call with 5 args to: {}",
                                                typed_source->to_string()) };
        }
      },
      source);
  }

  object_ptr dynamic_call(object_ptr const source,
                          object_ptr const a1,
                          object_ptr const a2,
                          object_ptr const a3,
                          object_ptr const a4,
                          object_ptr const a5,
                          object_ptr const a6)
  {
    return visit_object(
      [=](auto const typed_source) -> object_ptr {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(function_like<T> || std::is_base_of_v<callable, T>)
        {
          auto const arity_flags(typed_source->get_arity_flags());
          auto const mask(callable::extract_variadic_arity_mask(arity_flags));

          switch(mask)
          {
            case callable::mask_variadic_arity(0):
              return typed_source->call(
                make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6));
            case callable::mask_variadic_arity(1):
              return typed_source->call(a1,
                                        make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6));
            case callable::mask_variadic_arity(2):
              return typed_source->call(a1,
                                        a2,
                                        make_box<obj::native_array_sequence>(a3, a4, a5, a6));
            case callable::mask_variadic_arity(3):
              return typed_source->call(a1,
                                        a2,
                                        a3,
                                        make_box<obj::native_array_sequence>(a4, a5, a6));
            case callable::mask_variadic_arity(4):
              return typed_source->call(a1,
                                        a2,
                                        a3,
                                        a4,
                                        make_box<obj::native_array_sequence>(a5, a6));
            case callable::mask_variadic_arity(5):
              return typed_source
                ->call(a1, a2, a3, a4, a5, make_box<obj::native_array_sequence>(a6));
            case callable::mask_variadic_arity(6):
              if(!callable::is_variadic_ambiguous(arity_flags))
              {
                return typed_source->call(a1, a2, a3, a4, a5, a6, obj::nil::nil_const());
              }
            default:
              return typed_source->call(a1, a2, a3, a4, a5, a6);
          }
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid call with 6 args to: {}",
                                                typed_source->to_string()) };
        }
      },
      source);
  }

  object_ptr dynamic_call(object_ptr const source,
                          object_ptr const a1,
                          object_ptr const a2,
                          object_ptr const a3,
                          object_ptr const a4,
                          object_ptr const a5,
                          object_ptr const a6,
                          object_ptr const a7)
  {
    return visit_object(
      [=](auto const typed_source) -> object_ptr {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(function_like<T> || std::is_base_of_v<callable, T>)
        {
          auto const arity_flags(typed_source->get_arity_flags());
          auto const mask(callable::extract_variadic_arity_mask(arity_flags));

          switch(mask)
          {
            case callable::mask_variadic_arity(0):
              return typed_source->call(
                make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7));
            case callable::mask_variadic_arity(1):
              return typed_source->call(
                a1,
                make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7));
            case callable::mask_variadic_arity(2):
              return typed_source->call(a1,
                                        a2,
                                        make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7));
            case callable::mask_variadic_arity(3):
              return typed_source->call(a1,
                                        a2,
                                        a3,
                                        make_box<obj::native_array_sequence>(a4, a5, a6, a7));
            case callable::mask_variadic_arity(4):
              return typed_source->call(a1,
                                        a2,
                                        a3,
                                        a4,
                                        make_box<obj::native_array_sequence>(a5, a6, a7));
            case callable::mask_variadic_arity(5):
              return typed_source
                ->call(a1, a2, a3, a4, a5, make_box<obj::native_array_sequence>(a6, a7));
            case callable::mask_variadic_arity(6):
              return typed_source
                ->call(a1, a2, a3, a4, a5, a6, make_box<obj::native_array_sequence>(a7));
            case callable::mask_variadic_arity(7):
              if(!callable::is_variadic_ambiguous(arity_flags))
              {
                return typed_source->call(a1, a2, a3, a4, a5, a6, a7, obj::nil::nil_const());
              }
            default:
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7);
          }
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid call with 7 args to: {}",
                                                typed_source->to_string()) };
        }
      },
      source);
  }

  object_ptr dynamic_call(object_ptr const source,
                          object_ptr const a1,
                          object_ptr const a2,
                          object_ptr const a3,
                          object_ptr const a4,
                          object_ptr const a5,
                          object_ptr const a6,
                          object_ptr const a7,
                          object_ptr const a8)
  {
    return visit_object(
      [=](auto const typed_source) -> object_ptr {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(function_like<T> || std::is_base_of_v<callable, T>)
        {
          auto const arity_flags(typed_source->get_arity_flags());
          auto const mask(callable::extract_variadic_arity_mask(arity_flags));

          switch(mask)
          {
            case callable::mask_variadic_arity(0):
              return typed_source->call(
                make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7, a8));
            case callable::mask_variadic_arity(1):
              return typed_source->call(
                a1,
                make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7, a8));
            case callable::mask_variadic_arity(2):
              return typed_source->call(
                a1,
                a2,
                make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7, a8));
            case callable::mask_variadic_arity(3):
              return typed_source->call(a1,
                                        a2,
                                        a3,
                                        make_box<obj::native_array_sequence>(a4, a5, a6, a7, a8));
            case callable::mask_variadic_arity(4):
              return typed_source->call(a1,
                                        a2,
                                        a3,
                                        a4,
                                        make_box<obj::native_array_sequence>(a5, a6, a7, a8));
            case callable::mask_variadic_arity(5):
              return typed_source
                ->call(a1, a2, a3, a4, a5, make_box<obj::native_array_sequence>(a6, a7, a8));
            case callable::mask_variadic_arity(6):
              return typed_source
                ->call(a1, a2, a3, a4, a5, a6, make_box<obj::native_array_sequence>(a7, a8));
            case callable::mask_variadic_arity(7):
              return typed_source
                ->call(a1, a2, a3, a4, a5, a6, a7, make_box<obj::native_array_sequence>(a8));
            case callable::mask_variadic_arity(8):
              if(!callable::is_variadic_ambiguous(arity_flags))
              {
                return typed_source->call(a1, a2, a3, a4, a5, a6, a7, a8, obj::nil::nil_const());
              }
            default:
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, a8);
          }
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid call with 8 args to: {}",
                                                typed_source->to_string()) };
        }
      },
      source);
  }

  object_ptr dynamic_call(object_ptr const source,
                          object_ptr const a1,
                          object_ptr const a2,
                          object_ptr const a3,
                          object_ptr const a4,
                          object_ptr const a5,
                          object_ptr const a6,
                          object_ptr const a7,
                          object_ptr const a8,
                          object_ptr const a9)
  {
    return visit_object(
      [=](auto const typed_source) -> object_ptr {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(function_like<T> || std::is_base_of_v<callable, T>)
        {
          auto const arity_flags(typed_source->get_arity_flags());
          auto const mask(callable::extract_variadic_arity_mask(arity_flags));

          switch(mask)
          {
            case callable::mask_variadic_arity(0):
              return typed_source->call(
                make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7, a8, a9));
            case callable::mask_variadic_arity(1):
              return typed_source->call(
                a1,
                make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7, a8, a9));
            case callable::mask_variadic_arity(2):
              return typed_source->call(
                a1,
                a2,
                make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7, a8, a9));
            case callable::mask_variadic_arity(3):
              return typed_source
                ->call(a1, a2, a3, make_box<obj::native_array_sequence>(a4, a5, a6, a7, a8, a9));
            case callable::mask_variadic_arity(4):
              return typed_source->call(a1,
                                        a2,
                                        a3,
                                        a4,
                                        make_box<obj::native_array_sequence>(a5, a6, a7, a8, a9));
            case callable::mask_variadic_arity(5):
              return typed_source
                ->call(a1, a2, a3, a4, a5, make_box<obj::native_array_sequence>(a6, a7, a8, a9));
            case callable::mask_variadic_arity(6):
              return typed_source
                ->call(a1, a2, a3, a4, a5, a6, make_box<obj::native_array_sequence>(a7, a8, a9));
            case callable::mask_variadic_arity(7):
              return typed_source
                ->call(a1, a2, a3, a4, a5, a6, a7, make_box<obj::native_array_sequence>(a8, a9));
            case callable::mask_variadic_arity(8):
              return typed_source
                ->call(a1, a2, a3, a4, a5, a6, a7, a8, make_box<obj::native_array_sequence>(a9));
            case callable::mask_variadic_arity(9):
              if(!callable::is_variadic_ambiguous(arity_flags))
              {
                return typed_source
                  ->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, obj::nil::nil_const());
              }
            default:
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, a8, a9);
          }
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid call with 9 args to: {}",
                                                typed_source->to_string()) };
        }
      },
      source);
  }

  object_ptr dynamic_call(object_ptr const source,
                          object_ptr const a1,
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
    return visit_object(
      [=](auto const typed_source) -> object_ptr {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(function_like<T> || std::is_base_of_v<callable, T>)
        {
          auto const arity_flags(typed_source->get_arity_flags());
          auto const mask(callable::extract_variadic_arity_mask(arity_flags));

          switch(mask)
          {
            case callable::mask_variadic_arity(0):
              return typed_source->call(
                make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10));
            case callable::mask_variadic_arity(1):
              return typed_source->call(
                a1,
                make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7, a8, a9, a10));
            case callable::mask_variadic_arity(2):
              return typed_source->call(
                a1,
                a2,
                make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7, a8, a9, a10));
            case callable::mask_variadic_arity(3):
              return typed_source->call(
                a1,
                a2,
                a3,
                make_box<obj::native_array_sequence>(a4, a5, a6, a7, a8, a9, a10));
            case callable::mask_variadic_arity(4):
              return typed_source->call(
                a1,
                a2,
                a3,
                a4,
                make_box<obj::native_array_sequence>(a5, a6, a7, a8, a9, a10));
            case callable::mask_variadic_arity(5):
              return typed_source->call(a1,
                                        a2,
                                        a3,
                                        a4,
                                        a5,
                                        make_box<obj::native_array_sequence>(a6, a7, a8, a9, a10));
            case callable::mask_variadic_arity(6):
              return typed_source->call(a1,
                                        a2,
                                        a3,
                                        a4,
                                        a5,
                                        a6,
                                        make_box<obj::native_array_sequence>(a7, a8, a9, a10));
            case callable::mask_variadic_arity(7):
              return typed_source->call(a1,
                                        a2,
                                        a3,
                                        a4,
                                        a5,
                                        a6,
                                        a7,
                                        make_box<obj::native_array_sequence>(a8, a9, a10));
            case callable::mask_variadic_arity(8):
              return typed_source->call(a1,
                                        a2,
                                        a3,
                                        a4,
                                        a5,
                                        a6,
                                        a7,
                                        a8,
                                        make_box<obj::native_array_sequence>(a9, a10));
            case callable::mask_variadic_arity(9):
              return typed_source->call(a1,
                                        a2,
                                        a3,
                                        a4,
                                        a5,
                                        a6,
                                        a7,
                                        a8,
                                        a9,
                                        make_box<obj::native_array_sequence>(a10));
            case callable::mask_variadic_arity(10):
              if(!callable::is_variadic_ambiguous(arity_flags))
              {
                return typed_source->call(a1,
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
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
          }
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid call with 10 args to: {}",
                                                typed_source->to_string()) };
        }
      },
      source);
  }

  object_ptr dynamic_call(object_ptr const source,
                          object_ptr const a1,
                          object_ptr const a2,
                          object_ptr const a3,
                          object_ptr const a4,
                          object_ptr const a5,
                          object_ptr const a6,
                          object_ptr const a7,
                          object_ptr const a8,
                          object_ptr const a9,
                          object_ptr const a10,
                          obj::persistent_list_ptr const rest)
  {
    return visit_object(
      [=](auto const typed_source) -> object_ptr {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(function_like<T> || std::is_base_of_v<callable, T>)
        {
          auto const arity_flags(typed_source->get_arity_flags());
          auto const mask(callable::extract_variadic_arity_mask(arity_flags));
          switch(mask)
          {
            /* TODO: Optimize this with a faster seq? */
            case callable::mask_variadic_arity(0):
              {
                native_vector<object_ptr> packed;
                packed.reserve(10 + rest->count());
                packed.insert(packed.end(), { a1, a2, a3, a4, a5, a6, a7, a8, a9, a10 });
                std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
                return typed_source->call(make_box<obj::native_vector_sequence>(std::move(packed)));
              }
            case callable::mask_variadic_arity(1):
              {
                native_vector<object_ptr> packed;
                packed.reserve(9 + rest->count());
                packed.insert(packed.end(), { a2, a3, a4, a5, a6, a7, a8, a9, a10 });
                std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
                return typed_source->call(a1,
                                          make_box<obj::native_vector_sequence>(std::move(packed)));
              }
            case callable::mask_variadic_arity(2):
              {
                native_vector<object_ptr> packed;
                packed.reserve(8 + rest->count());
                packed.insert(packed.end(), { a3, a4, a5, a6, a7, a8, a9, a10 });
                std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
                return typed_source->call(a1,
                                          a2,
                                          make_box<obj::native_vector_sequence>(std::move(packed)));
              }
            case callable::mask_variadic_arity(3):
              {
                native_vector<object_ptr> packed;
                packed.reserve(7 + rest->count());
                packed.insert(packed.end(), { a4, a5, a6, a7, a8, a9, a10 });
                std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
                return typed_source->call(a1,
                                          a2,
                                          a3,
                                          make_box<obj::native_vector_sequence>(std::move(packed)));
              }
            case callable::mask_variadic_arity(4):
              {
                native_vector<object_ptr> packed;
                packed.reserve(6 + rest->count());
                packed.insert(packed.end(), { a5, a6, a7, a8, a9, a10 });
                std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
                return typed_source->call(a1,
                                          a2,
                                          a3,
                                          a4,
                                          make_box<obj::native_vector_sequence>(std::move(packed)));
              }
            case callable::mask_variadic_arity(5):
              {
                native_vector<object_ptr> packed;
                packed.reserve(5 + rest->count());
                packed.insert(packed.end(), { a6, a7, a8, a9, a10 });
                std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
                return typed_source->call(a1,
                                          a2,
                                          a3,
                                          a4,
                                          a5,
                                          make_box<obj::native_vector_sequence>(std::move(packed)));
              }
            case callable::mask_variadic_arity(6):
              {
                native_vector<object_ptr> packed;
                packed.reserve(4 + rest->count());
                packed.insert(packed.end(), { a7, a8, a9, a10 });
                std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
                return typed_source->call(a1,
                                          a2,
                                          a3,
                                          a4,
                                          a5,
                                          a6,
                                          make_box<obj::native_vector_sequence>(std::move(packed)));
              }
            case callable::mask_variadic_arity(7):
              {
                native_vector<object_ptr> packed;
                packed.reserve(3 + rest->count());
                packed.insert(packed.end(), { a8, a9, a10 });
                std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
                return typed_source->call(a1,
                                          a2,
                                          a3,
                                          a4,
                                          a5,
                                          a6,
                                          a7,
                                          make_box<obj::native_vector_sequence>(std::move(packed)));
              }
            case callable::mask_variadic_arity(8):
              {
                native_vector<object_ptr> packed;
                packed.reserve(2 + rest->count());
                packed.insert(packed.end(), { a9, a10 });
                std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
                return typed_source->call(a1,
                                          a2,
                                          a3,
                                          a4,
                                          a5,
                                          a6,
                                          a7,
                                          a8,
                                          make_box<obj::native_vector_sequence>(std::move(packed)));
              }
            case callable::mask_variadic_arity(9):
              {
                native_vector<object_ptr> packed;
                packed.reserve(1 + rest->count());
                packed.insert(packed.end(), { a10 });
                std::copy(rest->data.begin(), rest->data.end(), std::back_inserter(packed));
                return typed_source->call(a1,
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
              throw std::runtime_error{ fmt::format("unsupported arity: {}", 10 + rest->count()) };
          }
        }
        else
        {
          throw std::runtime_error{ fmt::format("invalid call with {} args to: {}",
                                                10 + runtime::detail::sequence_length(rest),
                                                typed_source->to_string()) };
        }
      },
      source);
  }

  object_ptr apply_to(object_ptr const source, object_ptr const args)
  {
    return visit_object(
      [=](auto const typed_args) -> object_ptr {
        using T = typename decltype(typed_args)::value_type;

        if constexpr(seqable<T>)
        {
          auto const s(typed_args->fresh_seq());
          auto const length(runtime::detail::sequence_length(s, max_params + 1));
          switch(length)
          {
            case 0:
              return dynamic_call(source);
            case 1:
              return dynamic_call(source, s->first());
            case 2:
              return dynamic_call(source, s->first(), s->next_in_place_first());
            case 3:
              return dynamic_call(source,
                                  s->first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first());
            case 4:
              return dynamic_call(source,
                                  s->first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first());
            case 5:
              return dynamic_call(source,
                                  s->first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first());
            case 6:
              return dynamic_call(source,
                                  s->first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first());
            case 7:
              return dynamic_call(source,
                                  s->first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first());
            case 8:
              return dynamic_call(source,
                                  s->first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first());
            case 9:
              return dynamic_call(source,
                                  s->first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first());
            case 10:
              return dynamic_call(source,
                                  s->first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first());
            default:
              return dynamic_call(source,
                                  s->first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  s->next_in_place_first(),
                                  obj::persistent_list::create(s->next_in_place()));
          }
        }
        else
        {
          throw std::runtime_error{ fmt::format("not seqable: {}", typed_args->to_string()) };
        }
      },
      args);
  }

  namespace behavior
  {
    object_ptr callable::call() const
    {
      throw invalid_arity<0>{};
    }

    object_ptr callable::call(object_ptr) const
    {
      throw invalid_arity<1>{};
    }

    object_ptr callable::call(object_ptr, object_ptr) const
    {
      throw invalid_arity<2>{};
    }

    object_ptr callable::call(object_ptr, object_ptr, object_ptr) const
    {
      throw invalid_arity<3>{};
    }

    object_ptr callable::call(object_ptr, object_ptr, object_ptr, object_ptr) const
    {
      throw invalid_arity<4>{};
    }

    object_ptr callable::call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const
    {
      throw invalid_arity<5>{};
    }

    object_ptr
    callable::call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const
    {
      throw invalid_arity<6>{};
    }

    object_ptr callable::call(object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr) const
    {
      throw invalid_arity<7>{};
    }

    object_ptr callable::call(object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr) const
    {
      throw invalid_arity<8>{};
    }

    object_ptr callable::call(object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr) const
    {
      throw invalid_arity<9>{};
    }

    object_ptr callable::call(object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr,
                              object_ptr) const
    {
      throw invalid_arity<10>{};
    }

    callable::arity_flag_t callable::get_arity_flags() const
    {
      return 0;
    }
  }
}
