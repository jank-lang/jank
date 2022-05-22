#include <iostream>

#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/obj/function.hpp>

namespace jank::runtime::behavior
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
}
