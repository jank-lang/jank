#include <jank/runtime/obj/tagged_literal.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/context.hpp>

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
    return runtime::equal(tag, s->tag) && runtime::equal(form, s->form);
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
    auto const tag_kw{ __rt_ctx->intern_keyword("tag").expect_ok() };
    auto const form_kw{ __rt_ctx->intern_keyword("form").expect_ok() };

    if(tag_kw == key)
    {
      return tag;
    }

    if(form_kw == key)
    {
      return form;
    }

    return fallback;
  }

  object_ptr tagged_literal::get(object_ptr const key) const
  {
    return get(key, obj::nil::nil_const());
  }

  object_ptr tagged_literal::get_entry(object_ptr const key) const
  {
    auto const tag_kw{ __rt_ctx->intern_keyword("tag").expect_ok() };
    auto const form_kw{ __rt_ctx->intern_keyword("form").expect_ok() };

    if(tag_kw == key)
    {
      return make_box<persistent_vector>(std::in_place, tag_kw, tag);
    }

    if(form_kw == key)
    {
      return make_box<persistent_vector>(std::in_place, form_kw, form);
    }

    return obj::nil::nil_const();
  }

  native_bool tagged_literal::contains(object_ptr const key) const
  {
    auto const tag_kw{ __rt_ctx->intern_keyword("tag").expect_ok() };
    auto const form_kw{ __rt_ctx->intern_keyword("form").expect_ok() };

    return tag_kw == key || form_kw == key;
  }
}
