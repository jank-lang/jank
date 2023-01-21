#include <iostream>
#include <sstream>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/set.hpp>

namespace jank::runtime::obj
{
  set::set(runtime::detail::persistent_set &&d)
    : data{ std::move(d) }
  { }
  set::set(runtime::detail::persistent_set const &d)
    : data{ d }
  { }

  ///***** set *****/
  //native_bool set::equal(object const &) const
  //{
  //}
  //native_string set::to_string() const
  //{
  //}
  //native_integer set::to_hash() const
  //{
  //}
  //set const* set::as_set() const
  //{ return this; }
  //iterator_ptr set::begin() const
  //{ return jank::make_box<basic_iterator_wrapper<detail::set_type::iterator>>(data.begin()); }
  //iterator_ptr set::end() const
  //{ return jank::make_box<basic_iterator_wrapper<detail::set_type::iterator>>(data.end()); }

  size_t set::count() const
  { return data.size(); }

  object_ptr set::with_meta(object_ptr m) const
  {
    validate_meta(m);
    auto ret(jank::make_box<set>(data));
    ret->meta = m;
    return ret;
  }

  behavior::metadatable const* set::as_metadatable() const
  { return this; }
}
