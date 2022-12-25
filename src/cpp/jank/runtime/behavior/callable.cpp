#include <iostream>

#include <fmt/core.h>

#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/list.hpp>
#include <jank/runtime/seq.hpp>
#include <jank/util/make_array.hpp>

namespace jank::runtime
{
  behavior::callable const* assert_callable(object_ptr const &source)
  {
    auto const * const c(source->as_callable());
    if(c == nullptr)
    { throw std::runtime_error{ "not callable: " + source->to_string().data }; }
    return c;
  }

  object_ptr dynamic_call(object_ptr const &source)
  {
    auto const * const c(assert_callable(source));
    auto const &variadic_arg_position(c->get_variadic_arg_position());
    switch(variadic_arg_position.unwrap_or(std::numeric_limits<size_t>::max()))
    {
      case 0:
        /* TODO: Empty list constant. */
        return c->call(make_box<obj::list>());
      default:
        return c->call();
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1)
  {
    auto const * const c(assert_callable(source));
    auto const &variadic_arg_position(c->get_variadic_arg_position());
    switch(variadic_arg_position.unwrap_or(std::numeric_limits<size_t>::max()))
    {
      case 0:
        return c->call(make_box<behavior::array_sequence<1>>(util::make_array(a1)));
      default:
        return c->call(a1);
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2)
  {
    auto const * const c(assert_callable(source));
    auto const &variadic_arg_position(c->get_variadic_arg_position());
    switch(variadic_arg_position.unwrap_or(std::numeric_limits<size_t>::max()))
    {
      case 0:
        return c->call(make_box<behavior::array_sequence<2>>(util::make_array(a1, a2)));
      case 1:
        return c->call(a1, make_box<behavior::array_sequence<1>>(util::make_array(a2)));
      default:
        return c->call(a1, a2);
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3)
  {
    auto const * const c(assert_callable(source));
    auto const &variadic_arg_position(c->get_variadic_arg_position());
    switch(variadic_arg_position.unwrap_or(std::numeric_limits<size_t>::max()))
    {
      case 0:
        return c->call(make_box<behavior::array_sequence<3>>(util::make_array(a1, a2, a3)));
      case 1:
        return c->call(a1, make_box<behavior::array_sequence<2>>(util::make_array(a2, a3)));
      case 2:
        return c->call(a1, a2, make_box<behavior::array_sequence<1>>(util::make_array(a3)));
      default:
        return c->call(a1, a2, a3);
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4)
  {
    auto const * const c(assert_callable(source));
    auto const &variadic_arg_position(c->get_variadic_arg_position());
    switch(variadic_arg_position.unwrap_or(std::numeric_limits<size_t>::max()))
    {
      case 0:
        return c->call(make_box<behavior::array_sequence<4>>(util::make_array(a1, a2, a3, a4)));
      case 1:
        return c->call(a1, make_box<behavior::array_sequence<3>>(util::make_array(a2, a3, a4)));
      case 2:
        return c->call(a1, a2, make_box<behavior::array_sequence<2>>(util::make_array(a3, a4)));
      case 3:
        return c->call(a1, a2, a3, make_box<behavior::array_sequence<1>>(util::make_array(a4)));
      default:
        return c->call(a1, a2, a3, a4);
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4, object_ptr const &a5)
  {
    auto const * const c(assert_callable(source));
    auto const &variadic_arg_position(c->get_variadic_arg_position());
    switch(variadic_arg_position.unwrap_or(std::numeric_limits<size_t>::max()))
    {
      case 0:
        return c->call(make_box<behavior::array_sequence<5>>(util::make_array(a1, a2, a3, a4, a5)));
      case 1:
        return c->call(a1, make_box<behavior::array_sequence<4>>(util::make_array(a2, a3, a4, a5)));
      case 2:
        return c->call(a1, a2, make_box<behavior::array_sequence<3>>(util::make_array(a3, a4, a5)));
      case 3:
        return c->call(a1, a2, a3, make_box<behavior::array_sequence<2>>(util::make_array(a4, a5)));
      case 4:
        return c->call(a1, a2, a3, a4, make_box<behavior::array_sequence<1>>(util::make_array(a5)));
      default:
        return c->call(a1, a2, a3, a4, a5);
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4, object_ptr const &a5, object_ptr const &a6)
  {
    auto const * const c(assert_callable(source));
    auto const &variadic_arg_position(c->get_variadic_arg_position());
    switch(variadic_arg_position.unwrap_or(std::numeric_limits<size_t>::max()))
    {
      case 0:
        return c->call(make_box<behavior::array_sequence<6>>(util::make_array(a1, a2, a3, a4, a5, a6)));
      case 1:
        return c->call(a1, make_box<behavior::array_sequence<5>>(util::make_array(a2, a3, a4, a5, a6)));
      case 2:
        return c->call(a1, a2, make_box<behavior::array_sequence<4>>(util::make_array(a3, a4, a5, a6)));
      case 3:
        return c->call(a1, a2, a3, make_box<behavior::array_sequence<3>>(util::make_array(a4, a5, a6)));
      case 4:
        return c->call(a1, a2, a3, a4, make_box<behavior::array_sequence<2>>(util::make_array(a5, a6)));
      case 5:
        return c->call(a1, a2, a3, a4, a5, make_box<behavior::array_sequence<1>>(util::make_array(a6)));
      default:
        return c->call(a1, a2, a3, a4, a5, a6);
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4, object_ptr const &a5, object_ptr const &a6, object_ptr const &a7)
  {
    auto const * const c(assert_callable(source));
    auto const &variadic_arg_position(c->get_variadic_arg_position());
    switch(variadic_arg_position.unwrap_or(std::numeric_limits<size_t>::max()))
    {
      case 0:
        return c->call(make_box<behavior::array_sequence<7>>(util::make_array(a1, a2, a3, a4, a5, a6, a7)));
      case 1:
        return c->call(a1, make_box<behavior::array_sequence<6>>(util::make_array(a2, a3, a4, a5, a6, a7)));
      case 2:
        return c->call(a1, a2, make_box<behavior::array_sequence<5>>(util::make_array(a3, a4, a5, a6, a7)));
      case 3:
        return c->call(a1, a2, a3, make_box<behavior::array_sequence<4>>(util::make_array(a4, a5, a6, a7)));
      case 4:
        return c->call(a1, a2, a3, a4, make_box<behavior::array_sequence<3>>(util::make_array(a5, a6, a7)));
      case 5:
        return c->call(a1, a2, a3, a4, a5, make_box<behavior::array_sequence<2>>(util::make_array(a6, a7)));
      case 6:
        return c->call(a1, a2, a3, a4, a5, a6, make_box<behavior::array_sequence<1>>(util::make_array(a7)));
      default:
        return c->call(a1, a2, a3, a4, a5, a6, a7);
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4, object_ptr const &a5, object_ptr const &a6, object_ptr const &a7, object_ptr const &a8)
  {
    auto const * const c(assert_callable(source));
    auto const &variadic_arg_position(c->get_variadic_arg_position());
    switch(variadic_arg_position.unwrap_or(std::numeric_limits<size_t>::max()))
    {
      case 0:
        return c->call(make_box<behavior::array_sequence<8>>(util::make_array(a1, a2, a3, a4, a5, a6, a7, a8)));
      case 1:
        return c->call(a1, make_box<behavior::array_sequence<7>>(util::make_array(a2, a3, a4, a5, a6, a7, a8)));
      case 2:
        return c->call(a1, a2, make_box<behavior::array_sequence<6>>(util::make_array(a3, a4, a5, a6, a7, a8)));
      case 3:
        return c->call(a1, a2, a3, make_box<behavior::array_sequence<5>>(util::make_array(a4, a5, a6, a7, a8)));
      case 4:
        return c->call(a1, a2, a3, a4, make_box<behavior::array_sequence<4>>(util::make_array(a5, a6, a7, a8)));
      case 5:
        return c->call(a1, a2, a3, a4, a5, make_box<behavior::array_sequence<3>>(util::make_array(a6, a7, a8)));
      case 6:
        return c->call(a1, a2, a3, a4, a5, a6, make_box<behavior::array_sequence<2>>(util::make_array(a7, a8)));
      case 7:
        return c->call(a1, a2, a3, a4, a5, a6, a7, make_box<behavior::array_sequence<1>>(util::make_array(a8)));
      default:
        return c->call(a1, a2, a3, a4, a5, a6, a7, a8);
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4, object_ptr const &a5, object_ptr const &a6, object_ptr const &a7, object_ptr const &a8, object_ptr const &a9)
  {
    auto const * const c(assert_callable(source));
    auto const &variadic_arg_position(c->get_variadic_arg_position());
    switch(variadic_arg_position.unwrap_or(std::numeric_limits<size_t>::max()))
    {
      case 0:
        return c->call(make_box<behavior::array_sequence<9>>(util::make_array(a1, a2, a3, a4, a5, a6, a7, a8, a9)));
      case 1:
        return c->call(a1, make_box<behavior::array_sequence<8>>(util::make_array(a2, a3, a4, a5, a6, a7, a8, a9)));
      case 2:
        return c->call(a1, a2, make_box<behavior::array_sequence<7>>(util::make_array(a3, a4, a5, a6, a7, a8, a9)));
      case 3:
        return c->call(a1, a2, a3, make_box<behavior::array_sequence<6>>(util::make_array(a4, a5, a6, a7, a8, a9)));
      case 4:
        return c->call(a1, a2, a3, a4, make_box<behavior::array_sequence<5>>(util::make_array(a5, a6, a7, a8, a9)));
      case 5:
        return c->call(a1, a2, a3, a4, a5, make_box<behavior::array_sequence<4>>(util::make_array(a6, a7, a8, a9)));
      case 6:
        return c->call(a1, a2, a3, a4, a5, a6, make_box<behavior::array_sequence<3>>(util::make_array(a7, a8, a9)));
      case 7:
        return c->call(a1, a2, a3, a4, a5, a6, a7, make_box<behavior::array_sequence<2>>(util::make_array(a8, a9)));
      case 8:
        return c->call(a1, a2, a3, a4, a5, a6, a7, a8, make_box<behavior::array_sequence<1>>(util::make_array(a9)));
      default:
        return c->call(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4, object_ptr const &a5, object_ptr const &a6, object_ptr const &a7, object_ptr const &a8, object_ptr const &a9, object_ptr const &a10)
  {
    auto const * const c(assert_callable(source));
    auto const &variadic_arg_position(c->get_variadic_arg_position());
    switch(variadic_arg_position.unwrap_or(std::numeric_limits<size_t>::max()))
    {
      case 0:
        return c->call(make_box<behavior::array_sequence<10>>(util::make_array(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)));
      case 1:
        return c->call(a1, make_box<behavior::array_sequence<9>>(util::make_array(a2, a3, a4, a5, a6, a7, a8, a9, a10)));
      case 2:
        return c->call(a1, a2, make_box<behavior::array_sequence<8>>(util::make_array(a3, a4, a5, a6, a7, a8, a9, a10)));
      case 3:
        return c->call(a1, a2, a3, make_box<behavior::array_sequence<7>>(util::make_array(a4, a5, a6, a7, a8, a9, a10)));
      case 4:
        return c->call(a1, a2, a3, a4, make_box<behavior::array_sequence<6>>(util::make_array(a5, a6, a7, a8, a9, a10)));
      case 5:
        return c->call(a1, a2, a3, a4, a5, make_box<behavior::array_sequence<5>>(util::make_array(a6, a7, a8, a9, a10)));
      case 6:
        return c->call(a1, a2, a3, a4, a5, a6, make_box<behavior::array_sequence<4>>(util::make_array(a7, a8, a9, a10)));
      case 7:
        return c->call(a1, a2, a3, a4, a5, a6, a7, make_box<behavior::array_sequence<3>>(util::make_array(a8, a9, a10)));
      case 8:
        return c->call(a1, a2, a3, a4, a5, a6, a7, a8, make_box<behavior::array_sequence<2>>(util::make_array(a9, a10)));
      case 9:
        return c->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, make_box<behavior::array_sequence<1>>(util::make_array(a10)));
      default:
        return c->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4, object_ptr const &a5, object_ptr const &a6, object_ptr const &a7, object_ptr const &a8, object_ptr const &a9, object_ptr const &a10, obj::list_ptr const &rest)
  {
    auto const * const c(assert_callable(source));
    auto const &variadic_arg_position(c->get_variadic_arg_position());
    switch(variadic_arg_position.unwrap_or(std::numeric_limits<size_t>::max()))
    {
      /* TODO: Optimize this with a faster seq? */
      case 0:
      {
        std::vector<object_ptr> packed;
        packed.reserve(10 + rest->count());
        packed.insert(packed.end(), { a1, a2, a3, a4, a5, a6, a7, a8, a9, a10 });
        packed.insert(packed.end(), rest->data.begin(), rest->data.end());
        return c->call(make_box<behavior::vector_sequence>(std::move(packed)));
      }
      case 1:
      {
        std::vector<object_ptr> packed;
        packed.reserve(10 + rest->count());
        packed.insert(packed.end(), { a2, a3, a4, a5, a6, a7, a8, a9, a10 });
        packed.insert(packed.end(), rest->data.begin(), rest->data.end());
        return c->call(a1, make_box<behavior::vector_sequence>(std::move(packed)));
      }
      case 2:
      {
        std::vector<object_ptr> packed;
        packed.reserve(10 + rest->count());
        packed.insert(packed.end(), { a3, a4, a5, a6, a7, a8, a9, a10 });
        packed.insert(packed.end(), rest->data.begin(), rest->data.end());
        return c->call(a1, a2, make_box<behavior::vector_sequence>(std::move(packed)));
      }
      case 3:
      {
        std::vector<object_ptr> packed;
        packed.reserve(10 + rest->count());
        packed.insert(packed.end(), { a4, a5, a6, a7, a8, a9, a10 });
        packed.insert(packed.end(), rest->data.begin(), rest->data.end());
        return c->call(a1, a2, a3, make_box<behavior::vector_sequence>(std::move(packed)));
      }
      case 4:
      {
        std::vector<object_ptr> packed;
        packed.reserve(10 + rest->count());
        packed.insert(packed.end(), { a5, a6, a7, a8, a9, a10 });
        packed.insert(packed.end(), rest->data.begin(), rest->data.end());
        return c->call(a1, a2, a3, a4, make_box<behavior::vector_sequence>(std::move(packed)));
      }
      case 5:
      {
        std::vector<object_ptr> packed;
        packed.reserve(10 + rest->count());
        packed.insert(packed.end(), { a6, a7, a8, a9, a10 });
        packed.insert(packed.end(), rest->data.begin(), rest->data.end());
        return c->call(a1, a2, a3, a4, a5, make_box<behavior::vector_sequence>(std::move(packed)));
      }
      case 6:
      {
        std::vector<object_ptr> packed;
        packed.reserve(10 + rest->count());
        packed.insert(packed.end(), { a7, a8, a9, a10 });
        packed.insert(packed.end(), rest->data.begin(), rest->data.end());
        return c->call(a1, a2, a3, a4, a5, a6, make_box<behavior::vector_sequence>(std::move(packed)));
      }
      case 7:
      {
        std::vector<object_ptr> packed;
        packed.reserve(10 + rest->count());
        packed.insert(packed.end(), { a8, a9, a10 });
        packed.insert(packed.end(), rest->data.begin(), rest->data.end());
        return c->call(a1, a2, a3, a4, a5, a6, a7, make_box<behavior::vector_sequence>(std::move(packed)));
      }
      case 8:
      {
        std::vector<object_ptr> packed;
        packed.reserve(10 + rest->count());
        packed.insert(packed.end(), { a9, a10 });
        packed.insert(packed.end(), rest->data.begin(), rest->data.end());
        return c->call(a1, a2, a3, a4, a5, a6, a7, a8, make_box<behavior::vector_sequence>(std::move(packed)));
      }
      case 9:
      {
        std::vector<object_ptr> packed;
        packed.reserve(10 + rest->count());
        packed.insert(packed.end(), { a10 });
        packed.insert(packed.end(), rest->data.begin(), rest->data.end());
        return c->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, make_box<behavior::vector_sequence>(std::move(packed)));
      }
      default:
        throw std::runtime_error{ fmt::format("unsupported arity: {}", 10 + rest->count()) };
    }
  }

  object_ptr apply_to(object_ptr const &source, object_ptr const &args)
  {
    auto const &s(args->as_seqable()->seq());
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

  namespace behavior
  {
    object_ptr callable::call() const
    { throw obj::invalid_arity<0>{}; }
    object_ptr callable::call(object_ptr const&) const
    { throw obj::invalid_arity<1>{}; }
    object_ptr callable::call(object_ptr const&, object_ptr const&) const
    { throw obj::invalid_arity<2>{}; }
    object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&) const
    { throw obj::invalid_arity<3>{}; }
    object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const
    { throw obj::invalid_arity<4>{}; }
    object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const
    { throw obj::invalid_arity<5>{}; }
    object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const
    { throw obj::invalid_arity<6>{}; }
    object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const
    { throw obj::invalid_arity<7>{}; }
    object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const
    { throw obj::invalid_arity<8>{}; }
    object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const
    { throw obj::invalid_arity<9>{}; }
    object_ptr callable::call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const
    { throw obj::invalid_arity<10>{}; }

    option<size_t> callable::get_variadic_arg_position() const
    { return none; }
  }
}
