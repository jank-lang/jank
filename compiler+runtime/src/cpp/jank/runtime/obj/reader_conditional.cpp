#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/obj/reader_conditional.hpp>
#include <jank/runtime/core/seq_ext.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/equal.hpp>

namespace jank::runtime::obj
{
  reader_conditional::reader_conditional()
    : object{ obj_type, obj_behaviors }
  {
  }

  reader_conditional::reader_conditional(persistent_list_ref f, boolean_ref s)
    : object{ obj_type, obj_behaviors }
    , form{ f }
    , splicing{ s }
  {
  }

  bool reader_conditional::equal(object const &o) const
  {
    if(o.type != object_type::reader_conditional)
    {
      return false;
    }

    auto const typed_o{ expect_object<obj::reader_conditional>(&o) };
    return runtime::equal(splicing, typed_o->splicing) && runtime::equal(form, typed_o->form);
  }

  static void to_string_impl(persistent_list_ref const form,
                             boolean_ref const splicing,
                             jtl::string_builder &buff)
  {
    buff(truthy(splicing) ? "#?@" : "#?");
    runtime::to_string(form, buff);
  }

  void reader_conditional::to_string(jtl::string_builder &buff) const
  {
    to_string_impl(form, splicing, buff);
  }

  jtl::immutable_string reader_conditional::to_string() const
  {
    jtl::string_builder buff;
    to_string_impl(form, splicing, buff);
    return buff.release();
  }

  jtl::immutable_string reader_conditional::to_code_string() const
  {
    return to_string();
  }

  object_ref reader_conditional::get(object_ref const key, object_ref const fallback) const
  {
    static auto const form_kw{ __rt_ctx->intern_keyword("form").expect_ok() };
    static auto const splicing_qmark_kw{ __rt_ctx->intern_keyword("splicing?").expect_ok() };

    if(runtime::equal(key, form_kw))
    {
      return form;
    }

    if(runtime::equal(key, splicing_qmark_kw))
    {
      return splicing;
    }

    return fallback;
  }

  object_ref reader_conditional::get(object_ref const key) const
  {
    return get(key, {});
  }

  bool reader_conditional::contains(object_ref const key) const
  {
    static auto const form_kw{ __rt_ctx->intern_keyword("form").expect_ok() };
    static auto const splicing_qmark_kw{ __rt_ctx->intern_keyword("splicing?").expect_ok() };

    return form_kw == key || splicing_qmark_kw == key;
  }

  /* TODO: Cache this. */
  uhash reader_conditional::to_hash() const
  {
    return hash::combine(splicing->to_hash(), form->to_hash());
  }
}
