#include <iostream>

#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/list.hpp>

namespace jank::runtime
{
  behavior::callable const* assert_callable(object_ptr const &source)
  {
    auto const * const c(source->as_callable());
    if(c == nullptr)
    { throw std::runtime_error{ "not callable: " + *source->to_string().data }; }
    return c;
  }

  /* TODO: Use required arity. */
  object_ptr dynamic_call(object_ptr const &source)
  {
    auto const * const c(assert_callable(source));
    auto const &required_arity(c->get_required_arity());
    if(required_arity.is_none())
    { return c->call(); }
    switch(required_arity.unwrap())
    {
      case 0:
        /* TODO: Empty list constant. */
        return c->call(runtime::obj::list::create());
      default:
        throw obj::invalid_arity<0>{};
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1)
  {
    auto const * const c(assert_callable(source));
    auto const &required_arity(c->get_required_arity());
    if(required_arity.is_none())
    { return c->call(a1); }
    switch(required_arity.unwrap())
    {
      case 0:
        return c->call(runtime::obj::list::create(a1));
      case 1:
        return c->call(a1, runtime::obj::list::create());
      default:
        throw obj::invalid_arity<1>{};
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2)
  {
    auto const * const c(assert_callable(source));
    auto const &required_arity(c->get_required_arity());
    if(required_arity.is_none())
    { return c->call(a1, a2); }
    switch(required_arity.unwrap())
    {
      case 0:
        return c->call(runtime::obj::list::create(a1, a2));
      case 1:
        return c->call(a1, runtime::obj::list::create(a2));
      case 2:
        return c->call(a1, a2, runtime::obj::list::create());
      default:
        throw obj::invalid_arity<2>{};
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3)
  {
    auto const * const c(assert_callable(source));
    auto const &required_arity(c->get_required_arity());
    if(required_arity.is_none())
    { return c->call(a1, a2, a3); }
    switch(required_arity.unwrap())
    {
      case 0:
        return c->call(runtime::obj::list::create(a1, a2, a3));
      case 1:
        return c->call(a1, runtime::obj::list::create(a2, a3));
      case 2:
        return c->call(a1, a2, runtime::obj::list::create(a3));
      case 3:
        return c->call(a1, a2, a3, runtime::obj::list::create());
      default:
        throw obj::invalid_arity<3>{};
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4)
  {
    auto const * const c(assert_callable(source));
    auto const &required_arity(c->get_required_arity());
    if(required_arity.is_none())
    { return c->call(a1, a2, a3, a4); }
    switch(required_arity.unwrap())
    {
      case 0:
        return c->call(runtime::obj::list::create(a1, a2, a3, a4));
      case 1:
        return c->call(a1, runtime::obj::list::create(a2, a3, a4));
      case 2:
        return c->call(a1, a2, runtime::obj::list::create(a3, a4));
      case 3:
        return c->call(a1, a2, a3, runtime::obj::list::create(a4));
      case 4:
        return c->call(a1, a2, a3, a4, runtime::obj::list::create());
      default:
        throw obj::invalid_arity<4>{};
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4, object_ptr const &a5)
  {
    auto const * const c(assert_callable(source));
    auto const &required_arity(c->get_required_arity());
    if(required_arity.is_none())
    { return c->call(a1, a2, a3, a4, a5); }
    switch(required_arity.unwrap())
    {
      case 0:
        return c->call(runtime::obj::list::create(a1, a2, a3, a4, a5));
      case 1:
        return c->call(a1, runtime::obj::list::create(a2, a3, a4, a5));
      case 2:
        return c->call(a1, a2, runtime::obj::list::create(a3, a4, a5));
      case 3:
        return c->call(a1, a2, a3, runtime::obj::list::create(a4, a5));
      case 4:
        return c->call(a1, a2, a3, a4, runtime::obj::list::create(a5));
      case 5:
        return c->call(a1, a2, a3, a4, a5, runtime::obj::list::create());
      default:
        throw obj::invalid_arity<5>{};
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4, object_ptr const &a5, object_ptr const &a6)
  {
    auto const * const c(assert_callable(source));
    auto const &required_arity(c->get_required_arity());
    if(required_arity.is_none())
    { return c->call(a1, a2, a3, a4, a5, a6); }
    switch(required_arity.unwrap())
    {
      case 0:
        return c->call(runtime::obj::list::create(a1, a2, a3, a4, a5, a6));
      case 1:
        return c->call(a1, runtime::obj::list::create(a2, a3, a4, a5, a6));
      case 2:
        return c->call(a1, a2, runtime::obj::list::create(a3, a4, a5, a6));
      case 3:
        return c->call(a1, a2, a3, runtime::obj::list::create(a4, a5, a6));
      case 4:
        return c->call(a1, a2, a3, a4, runtime::obj::list::create(a5, a6));
      case 5:
        return c->call(a1, a2, a3, a4, a5, runtime::obj::list::create(a6));
      case 6:
        return c->call(a1, a2, a3, a4, a5, a6, runtime::obj::list::create());
      default:
        throw obj::invalid_arity<6>{};
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4, object_ptr const &a5, object_ptr const &a6, object_ptr const &a7)
  {
    auto const * const c(assert_callable(source));
    auto const &required_arity(c->get_required_arity());
    if(required_arity.is_none())
    { return c->call(a1, a2, a3, a4, a5, a6, a7); }
    switch(required_arity.unwrap())
    {
      case 0:
        return c->call(runtime::obj::list::create(a1, a2, a3, a4, a5, a6, a7));
      case 1:
        return c->call(a1, runtime::obj::list::create(a2, a3, a4, a5, a6, a7));
      case 2:
        return c->call(a1, a2, runtime::obj::list::create(a3, a4, a5, a6, a7));
      case 3:
        return c->call(a1, a2, a3, runtime::obj::list::create(a4, a5, a6, a7));
      case 4:
        return c->call(a1, a2, a3, a4, runtime::obj::list::create(a5, a6, a7));
      case 5:
        return c->call(a1, a2, a3, a4, a5, runtime::obj::list::create(a6, a7));
      case 6:
        return c->call(a1, a2, a3, a4, a5, a6, runtime::obj::list::create(a7));
      case 7:
        return c->call(a1, a2, a3, a4, a5, a6, a7, runtime::obj::list::create());
      default:
        throw obj::invalid_arity<7>{};
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4, object_ptr const &a5, object_ptr const &a6, object_ptr const &a7, object_ptr const &a8)
  {
    auto const * const c(assert_callable(source));
    auto const &required_arity(c->get_required_arity());
    if(required_arity.is_none())
    { return c->call(a1, a2, a3, a4, a5, a6, a7, a8); }
    switch(required_arity.unwrap())
    {
      case 0:
        return c->call(runtime::obj::list::create(a1, a2, a3, a4, a5, a6, a7, a8));
      case 1:
        return c->call(a1, runtime::obj::list::create(a2, a3, a4, a5, a6, a7, a8));
      case 2:
        return c->call(a1, a2, runtime::obj::list::create(a3, a4, a5, a6, a7, a8));
      case 3:
        return c->call(a1, a2, a3, runtime::obj::list::create(a4, a5, a6, a7, a8));
      case 4:
        return c->call(a1, a2, a3, a4, runtime::obj::list::create(a5, a6, a7, a8));
      case 5:
        return c->call(a1, a2, a3, a4, a5, runtime::obj::list::create(a6, a7, a8));
      case 6:
        return c->call(a1, a2, a3, a4, a5, a6, runtime::obj::list::create(a7, a8));
      case 7:
        return c->call(a1, a2, a3, a4, a5, a6, a7, runtime::obj::list::create(a8));
      case 8:
        return c->call(a1, a2, a3, a4, a5, a6, a7, a8, runtime::obj::list::create());
      default:
        throw obj::invalid_arity<8>{};
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4, object_ptr const &a5, object_ptr const &a6, object_ptr const &a7, object_ptr const &a8, object_ptr const &a9)
  {
    auto const * const c(assert_callable(source));
    auto const &required_arity(c->get_required_arity());
    if(required_arity.is_none())
    { return c->call(a1, a2, a3, a4, a5, a6, a7, a8, a9); }
    switch(required_arity.unwrap())
    {
      case 0:
        return c->call(runtime::obj::list::create(a1, a2, a3, a4, a5, a6, a7, a8, a9));
      case 1:
        return c->call(a1, runtime::obj::list::create(a2, a3, a4, a5, a6, a7, a8, a9));
      case 2:
        return c->call(a1, a2, runtime::obj::list::create(a3, a4, a5, a6, a7, a8, a9));
      case 3:
        return c->call(a1, a2, a3, runtime::obj::list::create(a4, a5, a6, a7, a8, a9));
      case 4:
        return c->call(a1, a2, a3, a4, runtime::obj::list::create(a5, a6, a7, a8, a9));
      case 5:
        return c->call(a1, a2, a3, a4, a5, runtime::obj::list::create(a6, a7, a8, a9));
      case 6:
        return c->call(a1, a2, a3, a4, a5, a6, runtime::obj::list::create(a7, a8, a9));
      case 7:
        return c->call(a1, a2, a3, a4, a5, a6, a7, runtime::obj::list::create(a8, a9));
      case 8:
        return c->call(a1, a2, a3, a4, a5, a6, a7, a8, runtime::obj::list::create(a9));
      case 9:
        return c->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, runtime::obj::list::create());
      default:
        throw obj::invalid_arity<9>{};
    }
  }
  object_ptr dynamic_call(object_ptr const &source, object_ptr const &a1, object_ptr const &a2, object_ptr const &a3, object_ptr const &a4, object_ptr const &a5, object_ptr const &a6, object_ptr const &a7, object_ptr const &a8, object_ptr const &a9, object_ptr const &a10)
  {
    auto const * const c(assert_callable(source));
    auto const &required_arity(c->get_required_arity());
    if(required_arity.is_none())
    { return c->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10); }
    switch(required_arity.unwrap())
    {
      case 0:
        return c->call(runtime::obj::list::create(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10));
      case 1:
        return c->call(a1, runtime::obj::list::create(a2, a3, a4, a5, a6, a7, a8, a9, a10));
      case 2:
        return c->call(a1, a2, runtime::obj::list::create(a3, a4, a5, a6, a7, a8, a9, a10));
      case 3:
        return c->call(a1, a2, a3, runtime::obj::list::create(a4, a5, a6, a7, a8, a9, a10));
      case 4:
        return c->call(a1, a2, a3, a4, runtime::obj::list::create(a5, a6, a7, a8, a9, a10));
      case 5:
        return c->call(a1, a2, a3, a4, a5, runtime::obj::list::create(a6, a7, a8, a9, a10));
      case 6:
        return c->call(a1, a2, a3, a4, a5, a6, runtime::obj::list::create(a7, a8, a9, a10));
      case 7:
        return c->call(a1, a2, a3, a4, a5, a6, a7, runtime::obj::list::create(a8, a9, a10));
      case 8:
        return c->call(a1, a2, a3, a4, a5, a6, a7, a8, runtime::obj::list::create(a9, a10));
      case 9:
        return c->call(a1, a2, a3, a4, a5, a6, a7, a8, a9, runtime::obj::list::create(a10));
      default:
        throw obj::invalid_arity<10>{};
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

    option<size_t> callable::get_required_arity() const
    { return none; }
  }
}
