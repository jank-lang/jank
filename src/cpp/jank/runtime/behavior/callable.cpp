#include <iostream>

#include <fmt/core.h>

#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/native_array_sequence.hpp>
#include <jank/runtime/obj/native_vector_sequence.hpp>
#include <jank/runtime/obj/list.hpp>
#include <jank/runtime/seq.hpp>
#include <jank/util/make_array.hpp>

namespace jank::runtime
{
  object_ptr dynamic_call(object_ptr const source)
  {
    return visit_object
    (
      [=](auto const typed_source) -> object_ptr
      {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(behavior::function_like<T> || std::is_base_of_v<behavior::callable, T>)
        {
          auto const variadic_arg_position(typed_source->get_variadic_arg_position());
          switch(variadic_arg_position)
          {
            case 0:
              /* TODO: Empty list constant. */
              return typed_source->call(jank::make_box<obj::list>());
            default:
              return typed_source->call();
          }
        }
        else
        { throw std::runtime_error{ fmt::format("not callable: {}", typed_source->to_string()) }; }
      },
      source
    );
  }

  /* TODO: Benchmark difference of closure versus args. */
  object_ptr dynamic_call(object_ptr const source, object_ptr const a1)
  {
    return visit_object
    (
      [=](auto const typed_source) -> object_ptr
      {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(behavior::function_like<T> || std::is_base_of_v<behavior::callable, T>)
        {
          auto const variadic_arg_position(typed_source->get_variadic_arg_position());
          switch(variadic_arg_position)
          {
            case 0:
              return typed_source->call(jank::make_box<obj::native_array_sequence>(a1));
            default:
              return typed_source->call(a1);
          }
        }
        else
        { throw std::runtime_error{ fmt::format("not callable: {}", typed_source->to_string()) }; }
      },
      source
    );
  }
  object_ptr dynamic_call(object_ptr const source, object_ptr const a1, object_ptr const a2)
  {
    return visit_object
    (
      [=](auto const typed_source) -> object_ptr
      {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(behavior::function_like<T> || std::is_base_of_v<behavior::callable, T>)
        {
          auto const variadic_arg_position(typed_source->get_variadic_arg_position());
          switch(variadic_arg_position)
          {
            case 0:
              return typed_source->call(jank::make_box<obj::native_array_sequence>(a1, a2));
            case 1:
              return typed_source->call(a1, jank::make_box<obj::native_array_sequence>(a2));
            default:
              return typed_source->call(a1, a2);
          }
        }
        else
        { throw std::runtime_error{ fmt::format("not callable: {}", typed_source->to_string()) }; }
      },
      source
    );
  }
  object_ptr dynamic_call(object_ptr const source, object_ptr const a1, object_ptr const a2, object_ptr const a3)
  {
    return visit_object
    (
      [=](auto const typed_source) -> object_ptr
      {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(behavior::function_like<T> || std::is_base_of_v<behavior::callable, T>)
        {
          auto const variadic_arg_position(typed_source->get_variadic_arg_position());
          switch(variadic_arg_position)
          {
            case 0:
              return typed_source->call(jank::make_box<obj::native_array_sequence>(a1, a2, a3));
            case 1:
              return typed_source->call(a1, jank::make_box<obj::native_array_sequence>(a2, a3));
            case 2:
              return typed_source->call(a1, a2, jank::make_box<obj::native_array_sequence>(a3));
            default:
              return typed_source->call(a1, a2, a3);
          }
        }
        else
        { throw std::runtime_error{ fmt::format("not callable: {}", typed_source->to_string()) }; }
      },
      source
    );
  }
  object_ptr dynamic_call(object_ptr const source, object_ptr const a1, object_ptr const a2, object_ptr const a3, object_ptr const a4)
  {
    return visit_object
    (
      [=](auto const typed_source) -> object_ptr
      {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(behavior::function_like<T> || std::is_base_of_v<behavior::callable, T>)
        {
          auto const variadic_arg_position(typed_source->get_variadic_arg_position());
          switch(variadic_arg_position)
          {
            case 0:
              return typed_source->call(jank::make_box<obj::native_array_sequence>(a1, a2, a3, a4));
            case 1:
              return typed_source->call(a1, jank::make_box<obj::native_array_sequence>(a2, a3, a4));
            case 2:
              return typed_source->call(a1, a2, jank::make_box<obj::native_array_sequence>(a3, a4));
            case 3:
              return typed_source->call(a1, a2, a3, jank::make_box<obj::native_array_sequence>(a4));
            default:
              return typed_source->call(a1, a2, a3, a4);
          }
        }
        else
        { throw std::runtime_error{ fmt::format("not callable: {}", typed_source->to_string()) }; }
      },
      source
    );
  }
  object_ptr dynamic_call(object_ptr const source, object_ptr const a1, object_ptr const a2, object_ptr const a3, object_ptr const a4, object_ptr const a5)
  {
    return visit_object
    (
      [=](auto const typed_source) -> object_ptr
      {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(behavior::function_like<T> || std::is_base_of_v<behavior::callable, T>)
        {
          auto const variadic_arg_position(typed_source->get_variadic_arg_position());
          switch(variadic_arg_position)
          {
            case 0:
              return typed_source->call(jank::make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5));
            case 1:
              return typed_source->call(a1, jank::make_box<obj::native_array_sequence>(a2, a3, a4, a5));
            case 2:
              return typed_source->call(a1, a2, jank::make_box<obj::native_array_sequence>(a3, a4, a5));
            case 3:
              return typed_source->call(a1, a2, a3, jank::make_box<obj::native_array_sequence>(a4, a5));
            case 4:
              return typed_source->call(a1, a2, a3, a4, jank::make_box<obj::native_array_sequence>(a5));
            default:
              return typed_source->call(a1, a2, a3, a4, a5);
          }
        }
        else
        { throw std::runtime_error{ fmt::format("not callable: {}", typed_source->to_string()) }; }
      },
      source
    );
  }
  object_ptr dynamic_call(object_ptr const source, object_ptr const a1, object_ptr const a2, object_ptr const a3, object_ptr const a4, object_ptr const a5, object_ptr const a6)
  {
    return visit_object
    (
      [=](auto const typed_source) -> object_ptr
      {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(behavior::function_like<T> || std::is_base_of_v<behavior::callable, T>)
        {
          auto const variadic_arg_position(typed_source->get_variadic_arg_position());
          switch(variadic_arg_position)
          {
            case 0:
              return typed_source->call(jank::make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6));
            case 1:
              return typed_source->call(a1, jank::make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6));
            case 2:
              return typed_source->call(a1, a2, jank::make_box<obj::native_array_sequence>(a3, a4, a5, a6));
            case 3:
              return typed_source->call(a1, a2, a3, jank::make_box<obj::native_array_sequence>(a4, a5, a6));
            case 4:
              return typed_source->call(a1, a2, a3, a4, jank::make_box<obj::native_array_sequence>(a5, a6));
            case 5:
              return typed_source->call(a1, a2, a3, a4, a5, jank::make_box<obj::native_array_sequence>(a6));
            default:
              return typed_source->call(a1, a2, a3, a4, a5, a6);
          }
        }
        else
        { throw std::runtime_error{ fmt::format("not callable: {}", typed_source->to_string()) }; }
      },
      source
    );
  }
  object_ptr dynamic_call(object_ptr const source, object_ptr const a1, object_ptr const a2, object_ptr const a3, object_ptr const a4, object_ptr const a5, object_ptr const a6, object_ptr const a7)
  {
    return visit_object
    (
      [=](auto const typed_source) -> object_ptr
      {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(behavior::function_like<T> || std::is_base_of_v<behavior::callable, T>)
        {
          auto const variadic_arg_position(typed_source->get_variadic_arg_position());
          switch(variadic_arg_position)
          {
            case 0:
              return typed_source->call(jank::make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7));
            case 1:
              return typed_source->call(a1, jank::make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7));
            case 2:
              return typed_source->call(a1, a2, jank::make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7));
            case 3:
              return typed_source->call(a1, a2, a3, jank::make_box<obj::native_array_sequence>(a4, a5, a6, a7));
            case 4:
              return typed_source->call(a1, a2, a3, a4, jank::make_box<obj::native_array_sequence>(a5, a6, a7));
            case 5:
              return typed_source->call(a1, a2, a3, a4, a5, jank::make_box<obj::native_array_sequence>(a6, a7));
            case 6:
              return typed_source->call(a1, a2, a3, a4, a5, a6, jank::make_box<obj::native_array_sequence>(a7));
            default:
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7);
          }
        }
        else
        { throw std::runtime_error{ fmt::format("not callable: {}", typed_source->to_string()) }; }
      },
      source
    );
  }
  object_ptr dynamic_call(object_ptr const source, object_ptr const a1, object_ptr const a2, object_ptr const a3, object_ptr const a4, object_ptr const a5, object_ptr const a6, object_ptr const a7, object_ptr const a8)
  {
    return visit_object
    (
      [=](auto const typed_source) -> object_ptr
      {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(behavior::function_like<T> || std::is_base_of_v<behavior::callable, T>)
        {
          auto const variadic_arg_position(typed_source->get_variadic_arg_position());
          switch(variadic_arg_position)
          {
            case 0:
              return typed_source->call(jank::make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7, a8));
            case 1:
              return typed_source->call(a1, jank::make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7, a8));
            case 2:
              return typed_source->call(a1, a2, jank::make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7, a8));
            case 3:
              return typed_source->call(a1, a2, a3, jank::make_box<obj::native_array_sequence>(a4, a5, a6, a7, a8));
            case 4:
              return typed_source->call(a1, a2, a3, a4, jank::make_box<obj::native_array_sequence>(a5, a6, a7, a8));
            case 5:
              return typed_source->call(a1, a2, a3, a4, a5, jank::make_box<obj::native_array_sequence>(a6, a7, a8));
            case 6:
              return typed_source->call(a1, a2, a3, a4, a5, a6, jank::make_box<obj::native_array_sequence>(a7, a8));
            case 7:
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, jank::make_box<obj::native_array_sequence>(a8));
            default:
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, a8);
          }
        }
        else
        { throw std::runtime_error{ fmt::format("not callable: {}", typed_source->to_string()) }; }
      },
      source
    );
  }
  object_ptr dynamic_call(object_ptr const source, object_ptr const a1, object_ptr const a2, object_ptr const a3, object_ptr const a4, object_ptr const a5, object_ptr const a6, object_ptr const a7, object_ptr const a8, object_ptr const a9)
  {
    return visit_object
    (
      [=](auto const typed_source) -> object_ptr
      {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(behavior::function_like<T> || std::is_base_of_v<behavior::callable, T>)
        {
          auto const variadic_arg_position(typed_source->get_variadic_arg_position());
          switch(variadic_arg_position)
          {
            case 0:
              return typed_source->call(jank::make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7, a8, a9));
            case 1:
              return typed_source->call(a1, jank::make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7, a8, a9));
            case 2:
              return typed_source->call(a1, a2, jank::make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7, a8, a9));
            case 3:
              return typed_source->call(a1, a2, a3, jank::make_box<obj::native_array_sequence>(a4, a5, a6, a7, a8, a9));
            case 4:
              return typed_source->call(a1, a2, a3, a4, jank::make_box<obj::native_array_sequence>(a5, a6, a7, a8, a9));
            case 5:
              return typed_source->call(a1, a2, a3, a4, a5, jank::make_box<obj::native_array_sequence>(a6, a7, a8, a9));
            case 6:
              return typed_source->call(a1, a2, a3, a4, a5, a6, jank::make_box<obj::native_array_sequence>(a7, a8, a9));
            case 7:
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, jank::make_box<obj::native_array_sequence>(a8, a9));
            case 8:
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, a8, jank::make_box<obj::native_array_sequence>(a9));
            default:
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, a8, a9);
          }
        }
        else
        { throw std::runtime_error{ fmt::format("not callable: {}", typed_source->to_string()) }; }
      },
      source
    );
  }
  object_ptr dynamic_call(object_ptr const source, object_ptr const a1, object_ptr const a2, object_ptr const a3, object_ptr const a4, object_ptr const a5, object_ptr const a6, object_ptr const a7, object_ptr const a8, object_ptr const a9, object_ptr const a10)
  {
    return visit_object
    (
      [=](auto const typed_source) -> object_ptr
      {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(behavior::function_like<T> || std::is_base_of_v<behavior::callable, T>)
        {
          auto const variadic_arg_position(typed_source->get_variadic_arg_position());
          switch(variadic_arg_position)
          {
            case 0:
              return typed_source->call(jank::make_box<obj::native_array_sequence>(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10));
            case 1:
              return typed_source->call(a1, jank::make_box<obj::native_array_sequence>(a2, a3, a4, a5, a6, a7, a8, a9, a10));
            case 2:
              return typed_source->call(a1, a2, jank::make_box<obj::native_array_sequence>(a3, a4, a5, a6, a7, a8, a9, a10));
            case 3:
              return typed_source->call(a1, a2, a3, jank::make_box<obj::native_array_sequence>(a4, a5, a6, a7, a8, a9, a10));
            case 4:
              return typed_source->call(a1, a2, a3, a4, jank::make_box<obj::native_array_sequence>(a5, a6, a7, a8, a9, a10));
            case 5:
              return typed_source->call(a1, a2, a3, a4, a5, jank::make_box<obj::native_array_sequence>(a6, a7, a8, a9, a10));
            case 6:
              return typed_source->call(a1, a2, a3, a4, a5, a6, jank::make_box<obj::native_array_sequence>(a7, a8, a9, a10));
            case 7:
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, jank::make_box<obj::native_array_sequence>(a8, a9, a10));
            case 8:
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, a8, jank::make_box<obj::native_array_sequence>(a9, a10));
            case 9:
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, jank::make_box<obj::native_array_sequence>(a10));
            default:
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
          }
        }
        else
        { throw std::runtime_error{ fmt::format("not callable: {}", typed_source->to_string()) }; }
      },
      source
    );
  }
  object_ptr dynamic_call(object_ptr const source, object_ptr const a1, object_ptr const a2, object_ptr const a3, object_ptr const a4, object_ptr const a5, object_ptr const a6, object_ptr const a7, object_ptr const a8, object_ptr const a9, object_ptr const a10, obj::list_ptr rest)
  {
    return visit_object
    (
      [=](auto const typed_source) -> object_ptr
      {
        using T = typename decltype(typed_source)::value_type;

        if constexpr(behavior::function_like<T> || std::is_base_of_v<behavior::callable, T>)
        {
          auto const variadic_arg_position(typed_source->get_variadic_arg_position());
          switch(variadic_arg_position)
          {
            /* TODO: Optimize this with a faster seq? */
            case 0:
            {
              native_vector<object_ptr> packed;
              packed.reserve(10 + rest->count());
              packed.insert(packed.end(), { a1, a2, a3, a4, a5, a6, a7, a8, a9, a10 });
              std::copy(rest->data.begin(), rest->data.end(), packed.end());
              return typed_source->call(jank::make_box<obj::native_vector_sequence>(std::move(packed)));
            }
            case 1:
            {
              native_vector<object_ptr> packed;
              packed.reserve(10 + rest->count());
              packed.insert(packed.end(), { a2, a3, a4, a5, a6, a7, a8, a9, a10 });
              std::copy(rest->data.begin(), rest->data.end(), packed.end());
              return typed_source->call(a1, jank::make_box<obj::native_vector_sequence>(std::move(packed)));
            }
            case 2:
            {
              native_vector<object_ptr> packed;
              packed.reserve(10 + rest->count());
              packed.insert(packed.end(), { a3, a4, a5, a6, a7, a8, a9, a10 });
              std::copy(rest->data.begin(), rest->data.end(), packed.end());
              return typed_source->call(a1, a2, jank::make_box<obj::native_vector_sequence>(std::move(packed)));
            }
            case 3:
            {
              native_vector<object_ptr> packed;
              packed.reserve(10 + rest->count());
              packed.insert(packed.end(), { a4, a5, a6, a7, a8, a9, a10 });
              std::copy(rest->data.begin(), rest->data.end(), packed.end());
              return typed_source->call(a1, a2, a3, jank::make_box<obj::native_vector_sequence>(std::move(packed)));
            }
            case 4:
            {
              native_vector<object_ptr> packed;
              packed.reserve(10 + rest->count());
              packed.insert(packed.end(), { a5, a6, a7, a8, a9, a10 });
              std::copy(rest->data.begin(), rest->data.end(), packed.end());
              return typed_source->call(a1, a2, a3, a4, jank::make_box<obj::native_vector_sequence>(std::move(packed)));
            }
            case 5:
            {
              native_vector<object_ptr> packed;
              packed.reserve(10 + rest->count());
              packed.insert(packed.end(), { a6, a7, a8, a9, a10 });
              std::copy(rest->data.begin(), rest->data.end(), packed.end());
              return typed_source->call(a1, a2, a3, a4, a5, jank::make_box<obj::native_vector_sequence>(std::move(packed)));
            }
            case 6:
            {
              native_vector<object_ptr> packed;
              packed.reserve(10 + rest->count());
              packed.insert(packed.end(), { a7, a8, a9, a10 });
              std::copy(rest->data.begin(), rest->data.end(), packed.end());
              return typed_source->call(a1, a2, a3, a4, a5, a6, jank::make_box<obj::native_vector_sequence>(std::move(packed)));
            }
            case 7:
            {
              native_vector<object_ptr> packed;
              packed.reserve(10 + rest->count());
              packed.insert(packed.end(), { a8, a9, a10 });
              std::copy(rest->data.begin(), rest->data.end(), packed.end());
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, jank::make_box<obj::native_vector_sequence>(std::move(packed)));
            }
            case 8:
            {
              native_vector<object_ptr> packed;
              packed.reserve(10 + rest->count());
              packed.insert(packed.end(), { a9, a10 });
              std::copy(rest->data.begin(), rest->data.end(), packed.end());
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, a8, jank::make_box<obj::native_vector_sequence>(std::move(packed)));
            }
            case 9:
            {
              native_vector<object_ptr> packed;
              packed.reserve(10 + rest->count());
              packed.insert(packed.end(), { a10 });
              std::copy(rest->data.begin(), rest->data.end(), packed.end());
              return typed_source->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, jank::make_box<obj::native_vector_sequence>(std::move(packed)));
            }
            default:
              throw std::runtime_error{ fmt::format("unsupported arity: {}", 10 + rest->count()) };
          }
        }
        else
        { throw std::runtime_error{ fmt::format("not callable: {}", typed_source->to_string()) }; }
      },
      source
    );
  }

  object_ptr apply_to(object_ptr const source, object_ptr const args)
  {
    return visit_object
    (
      [=](auto const typed_args) -> object_ptr
      {
        using T = typename decltype(typed_args)::value_type;

        if constexpr(behavior::seqable<T>)
        {
          auto const s(typed_args->seq());
          auto const length(detail::sequence_length(s, max_params + 1));
          switch(length)
          {
            case 0:
              return dynamic_call(source);
            case 1:
              return dynamic_call(source, s->first());
            case 2:
              return dynamic_call(source, s->first(), s->next_in_place_first());
            case 3:
              return dynamic_call
              (
                source,
                s->first(),
                s->next_in_place_first(),
                s->next_in_place_first()
              );
            case 4:
              return dynamic_call
              (
                source,
                s->first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first()
              );
            case 5:
              return dynamic_call
              (
                source,
                s->first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first()
              );
            case 6:
              return dynamic_call
              (
                source,
                s->first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first()
              );
            case 7:
              return dynamic_call
              (
                source,
                s->first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first()
              );
            case 8:
              return dynamic_call
              (
                source,
                s->first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first()
              );
            case 9:
              return dynamic_call
              (
                source,
                s->first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first()
              );
            case 10:
              return dynamic_call
              (
                source,
                s->first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first(),
                s->next_in_place_first()
              );
            default:
              return dynamic_call
              (
                source,
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
                obj::list::create(s->next_in_place())
              );
          }
        }
        else
        { throw std::runtime_error{ fmt::format("not seqable: {}", typed_args->to_string()) }; }
      },
      args
    );
  }

  namespace behavior
  {
    object_ptr callable::call() const
    { throw invalid_arity<0>{}; }
    object_ptr callable::call(object_ptr) const
    { throw invalid_arity<1>{}; }
    object_ptr callable::call(object_ptr, object_ptr) const
    { throw invalid_arity<2>{}; }
    object_ptr callable::call(object_ptr, object_ptr, object_ptr) const
    { throw invalid_arity<3>{}; }
    object_ptr callable::call(object_ptr, object_ptr, object_ptr, object_ptr) const
    { throw invalid_arity<4>{}; }
    object_ptr callable::call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const
    { throw invalid_arity<5>{}; }
    object_ptr callable::call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const
    { throw invalid_arity<6>{}; }
    object_ptr callable::call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const
    { throw invalid_arity<7>{}; }
    object_ptr callable::call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const
    { throw invalid_arity<8>{}; }
    object_ptr callable::call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const
    { throw invalid_arity<9>{}; }
    object_ptr callable::call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const
    { throw invalid_arity<10>{}; }

    size_t callable::get_variadic_arg_position() const
    { return std::numeric_limits<size_t>::max(); }
  }
}
