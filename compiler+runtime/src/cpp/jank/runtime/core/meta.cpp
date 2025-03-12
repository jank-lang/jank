#include <fmt/format.h>

#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/make_box.hpp>
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

    auto const file(get(source, __rt_ctx->intern_keyword("file").expect_ok()));
    auto const start(get(source, __rt_ctx->intern_keyword("start").expect_ok()));
    auto const end(get(source, __rt_ctx->intern_keyword("end").expect_ok()));

    auto const start_offset(get(start, __rt_ctx->intern_keyword("offset").expect_ok()));
    auto const start_line(get(start, __rt_ctx->intern_keyword("line").expect_ok()));
    auto const start_col(get(start, __rt_ctx->intern_keyword("col").expect_ok()));

    auto const end_offset(get(end, __rt_ctx->intern_keyword("offset").expect_ok()));
    auto const end_line(get(end, __rt_ctx->intern_keyword("line").expect_ok()));
    auto const end_col(get(end, __rt_ctx->intern_keyword("col").expect_ok()));

    auto const macro_expansion(
      get(source, __rt_ctx->intern_keyword("macro-expansion").expect_ok()));

    return {
      to_string(file),
      { static_cast<size_t>(to_int(start_offset)),
              static_cast<size_t>(to_int(start_line)),
              static_cast<size_t>(to_int(start_col)) },
      {   static_cast<size_t>(to_int(end_offset)),
              static_cast<size_t>(to_int(end_line)),
              static_cast<size_t>(to_int(end_col))  },
      macro_expansion
    };
  }

  read::source object_source(object_ptr const o)
  {
    auto const meta(runtime::meta(o));
    if(meta == obj::nil::nil_const())
    {
      return read::source::unknown;
    }
    return meta_source(meta);
  }

  obj::persistent_hash_map_ptr
  source_to_meta(read::source_position const &start, read::source_position const &end)
  {
    return source_to_meta(__rt_ctx->intern_keyword("jank/source").expect_ok(), start, end);
  }

  obj::persistent_hash_map_ptr source_to_meta(object_ptr const key,
                                              read::source_position const &start,
                                              read::source_position const &end)
  {
    auto const file{ runtime::__rt_ctx->current_file_var->deref() };

    auto source{ obj::persistent_array_map::empty()->to_transient() };
    source = source->assoc_in_place(__rt_ctx->intern_keyword("file").expect_ok(), file);

    auto const start_map{ obj::persistent_array_map::create_unique(
      __rt_ctx->intern_keyword("offset").expect_ok(),
      make_box(start.offset),
      __rt_ctx->intern_keyword("line").expect_ok(),
      make_box(start.line),
      __rt_ctx->intern_keyword("col").expect_ok(),
      make_box(start.col)) };
    auto const end_map{ obj::persistent_array_map::create_unique(
      __rt_ctx->intern_keyword("offset").expect_ok(),
      make_box(end.offset),
      __rt_ctx->intern_keyword("line").expect_ok(),
      make_box(end.line),
      __rt_ctx->intern_keyword("col").expect_ok(),
      make_box(end.col)) };
    source = source->assoc_in_place(__rt_ctx->intern_keyword("start").expect_ok(), start_map);
    source = source->assoc_in_place(__rt_ctx->intern_keyword("end").expect_ok(), end_map);

    return obj::persistent_hash_map::create_unique(std::make_pair(key, source->to_persistent()));
  }

  object_ptr strip_source_from_meta(object_ptr const meta)
  {
    return dissoc(meta, __rt_ctx->intern_keyword("jank/source").expect_ok());
  }
}
