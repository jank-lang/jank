#include <iostream>
#include <sstream>

#include <jank/runtime/seq.hpp>
#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/set.hpp>

namespace jank::runtime::obj
{
  set::set(runtime::detail::set_type &&d)
    : data{ std::move(d) }
  { }
  set::set(runtime::detail::set_type const &d)
    : data{ d }
  { }

  ///***** set *****/
  //detail::boolean_type set::equal(object const &) const
  //{
  //}
  //detail::string_type set::to_string() const
  //{
  //}
  //detail::integer_type set::to_hash() const
  //{
  //}
  //set const* set::as_set() const
  //{ return this; }
  //seqable const* set::as_seqable() const
  //{ return this; }
  //iterator_ptr set::begin() const
  //{ return make_box<basic_iterator_wrapper<detail::set_type::iterator>>(data.begin()); }
  //iterator_ptr set::end() const
  //{ return make_box<basic_iterator_wrapper<detail::set_type::iterator>>(data.end()); }
}
