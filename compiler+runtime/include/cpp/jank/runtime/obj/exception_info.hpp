#pragma once

/* Windows defines this and it becomes an issue in unity builds. */
#ifdef exception_info
  #undef exception_info
#endif

#include <memory>

#include <jank/runtime/object.hpp>

/* NOLINTNEXTLINE(modernize-concat-nested-namespaces): Not doable, due to the inline. */
namespace cpptrace
{
  inline namespace v1
  {
    struct raw_trace;
    struct stacktrace;
  }
}

namespace jank::runtime::obj
{
  using exception_info_ref = oref<struct exception_info>;

  struct exception_info : object
  {
    static constexpr object_type obj_type{ object_type::exception_info };
    static constexpr object_behavior obj_behaviors{ object_behavior::none };
    static constexpr bool pointer_free{ false };

    exception_info(jtl::immutable_string const &message, object_ref const data);

    /* behavior::object_like */
    jtl::immutable_string to_code_string() const override;

    void resolve();
    obj::persistent_array_map_ref to_map() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    jtl::immutable_string message;
    object_ref data;
    exception_info_ref cause;
    std::unique_ptr<cpptrace::raw_trace> raw_trace;
    std::unique_ptr<cpptrace::stacktrace> resolved_trace;
  };
}
