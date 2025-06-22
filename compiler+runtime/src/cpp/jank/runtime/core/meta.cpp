#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime
{
  object_ref meta(object_ref const m)
  {
    if(m.is_nil())
    {
      return m;
    }

    return visit_object(
      [](auto const typed_m) -> object_ref {
        using T = typename decltype(typed_m)::value_type;

        if constexpr(behavior::metadatable<T>)
        {
          return typed_m->meta.unwrap_or(jank_nil);
        }
        else
        {
          return jank_nil;
        }
      },
      m);
  }

  object_ref with_meta(object_ref const o, object_ref const m)
  {
    return visit_object(
      [](auto const typed_o, object_ref const m) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::metadatable<T>)
        {
          return typed_o->with_meta(m);
        }
        else
        {
          throw std::runtime_error{ util::format("not metadatable: {} [{}]",
                                                 typed_o->to_code_string(),
                                                 object_type_str(typed_o->base.type)) };
        }
      },
      o,
      m);
  }

  /* This is the same as with_meta, but it gracefully handles the target
   * not supporting metadata. In that case, the target is returned and nothing
   * is done with the meta. */
  object_ref with_meta_graceful(object_ref const o, object_ref const m)
  {
    return visit_object(
      [](auto const typed_o, object_ref const m) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::metadatable<T>)
        {
          return typed_o->with_meta(m);
        }
        else
        {
          return typed_o;
        }
      },
      o,
      m);
  }

  object_ref reset_meta(object_ref const o, object_ref const m)
  {
    return visit_object(
      [](auto const typed_o, object_ref const m) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(behavior::metadatable<T>)
        {
          auto const meta(behavior::detail::validate_meta(m));
          typed_o->meta = meta;
          return m;
        }
        else
        {
          throw std::runtime_error{ util::format("not metadatable: {} [{}]",
                                                 typed_o->to_code_string(),
                                                 object_type_str(typed_o->base.type)) };
        }
      },
      o,
      m);
  }

  read::source meta_source(jtl::option<runtime::object_ref> const &o)
  {
    using namespace jank::runtime;

    auto const meta(o.unwrap_or(jank_nil));
    auto const source(get(meta, __rt_ctx->intern_keyword("jank/source").expect_ok()));
    if(source == jank_nil)
    {
      return read::source::unknown;
    }

    auto const file(get(source, __rt_ctx->intern_keyword("file").expect_ok()));
    if(file == jank_nil)
    {
      return read::source::unknown;
    }

    auto const start(get(source, __rt_ctx->intern_keyword("start").expect_ok()));
    auto const end(get(source, __rt_ctx->intern_keyword("end").expect_ok()));

    auto const start_offset(get(start, __rt_ctx->intern_keyword("offset").expect_ok()));
    auto const start_line(get(start, __rt_ctx->intern_keyword("line").expect_ok()));
    auto const start_col(get(start, __rt_ctx->intern_keyword("col").expect_ok()));

    auto const end_offset(get(end, __rt_ctx->intern_keyword("offset").expect_ok()));
    auto const end_line(get(end, __rt_ctx->intern_keyword("line").expect_ok()));
    auto const end_col(get(end, __rt_ctx->intern_keyword("col").expect_ok()));

    auto const macro_expansion(
      get(meta, __rt_ctx->intern_keyword("jank/macro-expansion").expect_ok()));

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

  read::source object_source(object_ref const o)
  {
    auto const meta(runtime::meta(o));
    if(meta == jank_nil)
    {
      return read::source::unknown;
    }
    return meta_source(meta);
  }

  obj::persistent_hash_map_ref
  source_to_meta(read::source_position const &start, read::source_position const &end)
  {
    return source_to_meta(__rt_ctx->intern_keyword("jank/source").expect_ok(), start, end);
  }

  obj::persistent_hash_map_ref source_to_meta(object_ref const key,
                                              read::source_position const &start,
                                              read::source_position const &end)
  {
    auto const file{ runtime::__rt_ctx->current_file_var->deref() };

    auto source{ obj::persistent_array_map::empty()->to_transient() };
    source->assoc_in_place(__rt_ctx->intern_keyword("file").expect_ok(), file);

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
    source->assoc_in_place(__rt_ctx->intern_keyword("start").expect_ok(), start_map);
    source->assoc_in_place(__rt_ctx->intern_keyword("end").expect_ok(), end_map);

    return obj::persistent_hash_map::create_unique(std::make_pair(key, source->to_persistent()));
  }

  object_ref strip_source_from_meta(object_ref const meta)
  {
    return dissoc(meta, __rt_ctx->intern_keyword("jank/source").expect_ok());
  }
}
