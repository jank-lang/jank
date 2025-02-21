#include <fmt/format.h>

#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/native_persistent_string/fmt.hpp>

namespace jank::runtime
{
  object_ptr meta(object_ptr const m)
  {
    if(m == nullptr || m == obj::nil::nil_const())
    {
      return obj::nil::nil_const();
    }

    return visit_object(
      [](auto const typed_m) -> object_ptr {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::metadatable<T>)
        {
          return typed_m->meta.unwrap_or(obj::nil::nil_const());
        }
        else
        {
          return obj::nil::nil_const();
        }
      },
      m);
  }

  object_ptr with_meta(object_ptr const o, object_ptr const m)
  {
    return visit_object(
      [](auto const typed_o, object_ptr const m) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::metadatable<T>)
        {
          return typed_o->with_meta(m);
        }
        else
        {
          throw std::runtime_error{ fmt::format("not metadatable: {} [{}]",
                                                typed_o->to_code_string(),
                                                object_type_str(typed_o->base.type)) };
        }
      },
      o,
      m);
  }

  object_ptr reset_meta(object_ptr const o, object_ptr const m)
  {
    return visit_object(
      [](auto const typed_o, object_ptr const m) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::metadatable<T>)
        {
          auto const meta(behavior::detail::validate_meta(m));
          typed_o->meta = meta;
          return m;
        }
        else
        {
          throw std::runtime_error{ fmt::format("not metadatable: {} [{}]",
                                                typed_o->to_code_string(),
                                                object_type_str(typed_o->base.type)) };
        }
      },
      o,
      m);
  }

  read::source meta_source(option<runtime::object_ptr> const &o)
  {
    using namespace jank::runtime;

    auto const meta(o.unwrap_or(obj::nil::nil_const()));
    auto const source(get(meta, __rt_ctx->intern_keyword("jank/source").expect_ok()));
    if(source == obj::nil::nil_const())
    {
      return read::source::unknown;
    }

    auto const file{ runtime::__rt_ctx->current_file_var->deref() };
    auto const start(get(source, __rt_ctx->intern_keyword("start").expect_ok()));
    auto const end(get(source, __rt_ctx->intern_keyword("end").expect_ok()));

    auto const start_offset(get(start, __rt_ctx->intern_keyword("offset").expect_ok()));
    auto const start_line(get(start, __rt_ctx->intern_keyword("line").expect_ok()));
    auto const start_col(get(start, __rt_ctx->intern_keyword("col").expect_ok()));

    auto const end_offset(get(end, __rt_ctx->intern_keyword("offset").expect_ok()));
    auto const end_line(get(end, __rt_ctx->intern_keyword("line").expect_ok()));
    auto const end_col(get(end, __rt_ctx->intern_keyword("col").expect_ok()));

    return {
      to_string(file),
      { static_cast<size_t>(to_int(start_offset)),
              static_cast<size_t>(to_int(start_line)),
              static_cast<size_t>(to_int(start_col)) },
      {   static_cast<size_t>(to_int(end_offset)),
              static_cast<size_t>(to_int(end_line)),
              static_cast<size_t>(to_int(end_col))  }
    };
  }

  read::source object_source(runtime::object_ptr const o)
  {
    auto const meta(runtime::meta(o));
    if(meta == obj::nil::nil_const())
    {
      return read::source::unknown;
    }
    return meta_source(meta);
  }
}
