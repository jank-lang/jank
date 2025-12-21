#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/core/math.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/util/fmt.hpp>
#include <jank/error/runtime.hpp>

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
        using T = typename jtl::decay_t<decltype(typed_m)>::value_type;

        if constexpr(behavior::metadatable<T>)
        {
          return typed_m->meta.unwrap_or(jank_nil());
        }
        else
        {
          return jank_nil();
        }
      },
      m);
  }

  object_ref with_meta(object_ref const o, object_ref const m)
  {
    return visit_object(
      [&o](auto const typed_o, object_ref const m) -> object_ref {
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

        if constexpr(behavior::metadatable<T>)
        {
          return typed_o->with_meta(m);
        }
        else
        {
          throw error::runtime_non_metadatable_value(
            util::format("{} [{}] can't hold any metadata.",
                         typed_o->to_code_string(),
                         object_type_str(o->type)),
            object_source(o));
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
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

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
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

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

    auto const meta(o.unwrap_or(jank_nil()));
    auto const source(get(meta, __rt_ctx->intern_keyword("jank/source").expect_ok()));
    if(source == jank_nil())
    {
      return read::source::unknown();
    }

    auto const file(get(source, __rt_ctx->intern_keyword("file").expect_ok()));
    if(file == jank_nil())
    {
      return read::source::unknown();
    }

    auto const module(get(source, __rt_ctx->intern_keyword("module").expect_ok()));

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
      runtime::to_string(file),
      module.is_some() ? to_string(module) : "",
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
    if(meta == jank_nil())
    {
      return read::source::unknown();
    }
    return meta_source(meta);
  }

  obj::persistent_hash_map_ref source_to_meta(read::source const &source)
  {
    auto const source_map{ obj::persistent_array_map::empty()->to_transient() };

    if(runtime::module::is_core_module(source.module))
    {
      source_map->assoc_in_place(__rt_ctx->intern_keyword("module").expect_ok(),
                                 make_box(source.module));
    }

    if(source.file != read::no_source_path)
    {
      source_map->assoc_in_place(__rt_ctx->intern_keyword("file").expect_ok(),
                                 make_box(source.file));
    }

    auto const start_map{ obj::persistent_array_map::create_unique(
      __rt_ctx->intern_keyword("offset").expect_ok(),
      make_box(source.start.offset),
      __rt_ctx->intern_keyword("line").expect_ok(),
      make_box(source.start.line),
      __rt_ctx->intern_keyword("col").expect_ok(),
      make_box(source.start.col)) };
    auto const end_map{ obj::persistent_array_map::create_unique(
      __rt_ctx->intern_keyword("offset").expect_ok(),
      make_box(source.end.offset),
      __rt_ctx->intern_keyword("line").expect_ok(),
      make_box(source.end.line),
      __rt_ctx->intern_keyword("col").expect_ok(),
      make_box(source.end.col)) };
    source_map->assoc_in_place(__rt_ctx->intern_keyword("start").expect_ok(), start_map);
    source_map->assoc_in_place(__rt_ctx->intern_keyword("end").expect_ok(), end_map);

    auto const key{ __rt_ctx->intern_keyword("jank/source").expect_ok() };
    return obj::persistent_hash_map::create_unique(
      std::make_pair(key, source_map->to_persistent()));
  }

  obj::persistent_hash_map_ref
  source_to_meta(read::source_position const &start, read::source_position const &end)
  {
    read::source source{ start, end };

    auto const module{ runtime::to_code_string(runtime::__rt_ctx->current_ns_var->deref()) };
    if(runtime::module::is_core_module(module))
    {
      source.module = module;
    }

    auto const file{ runtime::__rt_ctx->current_file_var->deref() };
    source.file = runtime::to_string(file);

    return source_to_meta(source);
  }

  object_ref strip_source_from_meta(object_ref const meta)
  {
    auto const kw{ __rt_ctx->intern_keyword("jank/source").expect_ok() };
    return dissoc(meta, kw);
  }

  jtl::option<object_ref> strip_source_from_meta_opt(jtl::option<object_ref> const &meta)
  {
    if(meta.is_none())
    {
      return meta;
    }

    auto stripped{ strip_source_from_meta(meta.unwrap()) };
    if(is_empty(stripped))
    {
      return none;
    }
    return stripped;
  }
}
