#include <cpptrace/basic.hpp>

#include <jank/runtime/obj/exception_info.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/detail/type.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/util/try.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  exception_info::exception_info(jtl::immutable_string const &message, object_ref const data)
    : object{ obj_type, obj_behaviors }
    , message{ message }
    , data{ data }
  {
  }

  static persistent_vector_ref frame_to_vec(cpptrace::stacktrace_frame const &frame)
  {
    runtime::detail::native_transient_vector trans;
    trans.push_back(make_box(frame.symbol));
    trans.push_back(make_box(frame.filename));
    trans.push_back(make_box(frame.line.value_or(1)));
    return make_box<obj::persistent_vector>(trans.persistent());
  }

  static persistent_hash_map_ref cause_to_via(exception_info_ref const e)
  {
    static auto const message_kw{ __rt_ctx->intern_keyword("", "message").expect_ok() };
    static auto const data_kw{ __rt_ctx->intern_keyword("", "data").expect_ok() };
    static auto const at_kw{ __rt_ctx->intern_keyword("", "at").expect_ok() };

    e->resolve();

    runtime::detail::native_transient_hash_map trans{};
    trans.set(message_kw, make_box(e->message));
    trans.set(data_kw, e->data);
    if(e->resolved_trace)
    {
      trans.set(at_kw, frame_to_vec(e->resolved_trace->frames.at(0)));
    }

    return make_box<obj::persistent_hash_map>(trans.persistent());
  }

  jtl::immutable_string exception_info::to_code_string() const
  {
    const_cast<exception_info *>(this)->resolve();

    jtl::string_builder sb;
    util::format_to(sb, "#error {\n");
    util::format_to(sb, " :cause \"{}\"\n", message);
    util::format_to(sb, " :data {}\n", data.to_code_string());
    util::format_to(sb, " :via\n [");
    util::format_to(sb, "{}", cause_to_via(runtime::detail::untagged(this)).to_code_string());
    for(exception_info_ref cause_it{ cause }; cause_it.is_some(); cause_it = cause_it->cause)
    {
      util::format_to(sb, "\n  {}", cause_to_via(cause).to_code_string());
    }
    util::format_to(sb, "]\n");
    util::format_to(sb, " :trace\n [");
    bool needs_indent{};
    for(auto const &frame : resolved_trace->frames)
    {
      if(needs_indent)
      {
        util::format_to(sb, "\n  ");
      }

      util::format_to(sb, "{}", frame_to_vec(frame).to_code_string());

      needs_indent = true;
    }
    util::format_to(sb, "]}");
    return sb.release();
  }

  void exception_info::resolve()
  {
    if(resolved_trace.get() || !raw_trace)
    {
      return;
    }

    resolved_trace = std::make_unique<cpptrace::stacktrace>(util::resolve(*raw_trace));
    raw_trace.reset();
  }

  obj::persistent_array_map_ref exception_info::to_map() const
  {
    static auto const cause_kw{ __rt_ctx->intern_keyword("", "cause").expect_ok() };
    static auto const data_kw{ __rt_ctx->intern_keyword("", "data").expect_ok() };
    static auto const trace_kw{ __rt_ctx->intern_keyword("", "trace").expect_ok() };
    static auto const via_kw{ __rt_ctx->intern_keyword("", "via").expect_ok() };

    const_cast<exception_info *>(this)->resolve();

    runtime::detail::native_array_map trans{};

    trans.insert_unique(cause_kw, make_box(message));
    trans.insert_unique(data_kw, data);

    runtime::detail::native_transient_vector via_trans;
    for(exception_info_ref cause_it{ runtime::detail::untagged(this) }; cause_it.is_some();
        cause_it = cause_it->cause)
    {
      via_trans.push_back(cause_to_via(cause));
    }
    trans.insert_unique(via_kw, make_box<obj::persistent_vector>(via_trans.persistent()));

    runtime::detail::native_transient_vector trace_trans;
    for(auto const &frame : resolved_trace->frames)
    {
      trace_trans.push_back(frame_to_vec(frame));
    }
    trans.insert_unique(trace_kw, make_box<obj::persistent_vector>(trace_trans.persistent()));

    return make_box<obj::persistent_array_map>(trans);
  }
}
