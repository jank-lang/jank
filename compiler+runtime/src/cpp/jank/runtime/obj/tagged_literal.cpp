#include <jank/runtime/obj/tagged_literal.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/equal.hpp>

namespace jank::runtime::obj
{
  tagged_literal::tagged_literal(object_ref const tag, object_ref const form)
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
  to_string_impl(object_ref const &tag, object_ref const &form, util::string_builder &buff)
  {
    buff('#');
    runtime::to_string(tag, buff);
    buff(' ');
    runtime::to_string(form, buff);
  }

  void tagged_literal::to_string(util::string_builder &buff) const
  {
    to_string_impl(tag, form, buff);
  }

  jtl::immutable_string tagged_literal::to_string() const
  {
    util::string_builder buff;
    to_string_impl(tag, form, buff);
    return buff.release();
  }

  jtl::immutable_string tagged_literal::to_code_string() const
  {
    return to_string();
  }

  native_hash tagged_literal::to_hash() const
  {
    if(hash)
    {
      return hash;
    }

    return hash = hash::combine(hash::visit(tag.get()), hash::visit(form.get()));
  }

  object_ref tagged_literal::get(object_ref const key, object_ref const fallback) const
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

  object_ref tagged_literal::get(object_ref const key) const
  {
    return get(key, obj::nil::nil_const());
  }

  object_ref tagged_literal::get_entry(object_ref const key) const
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

  native_bool tagged_literal::contains(object_ref const key) const
  {
    auto const tag_kw{ __rt_ctx->intern_keyword("tag").expect_ok() };
    auto const form_kw{ __rt_ctx->intern_keyword("form").expect_ok() };

    return tag_kw == key || form_kw == key;
  }
}
