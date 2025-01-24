#include <jank/runtime/obj/symbol.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/context.hpp>
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
    if(o.type != object_type::tagged_literal)
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

  object_ptr tagged_literal::get(object_ptr const key, object_ptr const fallback) const
  {
    object_ptr _tag = __rt_ctx->intern_keyword("tag").expect_ok();
    object_ptr _form = __rt_ctx->intern_keyword("form").expect_ok();

    if(_tag == key)
    {
      return tag;
    }

    if(_form == key)
    {
      return form;
    }

    return fallback;
  }

  object_ptr tagged_literal::get(object_ptr const key) const
  {
    return get(key, obj::nil::nil_const());
  }

  object_ptr tagged_literal::get_entry(object_ptr /*key*/) const
  {
    return obj::nil::nil_const();
  }

  native_bool tagged_literal::contains(object_ptr /*key*/) const
  {
    return false;
  }

  native_bool is_tagged_literal(object_ptr const o)
  {
    return o->type == object_type::tagged_literal;
  }
}
