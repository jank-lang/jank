#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/obj/tagged_literal.hpp>

namespace jank::runtime::obj
{
  tagged_literal::tagged_literal(object_ptr const tag, object_ptr const form)
    : tag{ tag }
    , form{ form }
  {
  }

  native_bool tagged_literal::equal(object const &o) const
  {
    if(o.type != object_type::symbol)
    {
      return false;
    }

    auto const s(expect_object<tagged_literal>(&o));
    return tag == s->tag && form == s->form;
  }

  static void
  to_string_impl(object_ptr const &tag, object_ptr const &form, util::string_builder &buff)
  {
    buff('#');
    to_string(tag, buff);
    buff(' ');
    to_string(form, buff);
  }

  void tagged_literal::to_string(util::string_builder &buff) const
  {
    to_string_impl(tag, form, buff);
  }

  native_persistent_string tagged_literal::to_string() const
  {
    util::string_builder buff;
    to_string_impl(tag, form, buff);
    return buff.release();
  }

  native_persistent_string tagged_literal::to_code_string() const
  {
    return to_string();
  }

  native_hash tagged_literal::to_hash() const
  {
    if(hash)
    {
      return hash;
    }

    return hash = hash::combine(hash::visit(tag), hash::visit(form));
  }

  native_bool is_tagged_literal(object_ptr const o)
  {
    return o->type == object_type::tagged_literal;
  }
}
