#include <jank/error.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/to_string.hpp>

namespace jank::error
{
  native_bool base::operator==(base const &rhs) const
  {
    return !(*this != rhs);
  }

  native_bool base::operator!=(base const &rhs) const
  {
    return kind != rhs.kind || source != rhs.source || message != rhs.message;
  }

  std::ostream &operator<<(std::ostream &os, base const &e)
  {
    return os << "error(" << kind_str(e.kind) << " - " << e.source << ", \"" << e.message << "\")";
  }
}

namespace jank
{
  error_ptr make_error(error::kind const kind, native_persistent_string const &message)
  {
    auto const file{ runtime::__rt_ctx->current_file_var->deref() };
    return runtime::make_box<error::base>(gc{},
                                          kind,
                                          message,
                                          read::source{ runtime::to_string(file), {}, {} });
  }

  error_ptr make_error(error::kind const kind,
                       native_persistent_string const &message,
                       read::source_position const &start)
  {
    auto const file{ runtime::__rt_ctx->current_file_var->deref() };
    return runtime::make_box<error::base>(gc{},
                                          kind,
                                          message,
                                          read::source{ runtime::to_string(file), start, start });
  }

  error_ptr make_error(error::kind const kind,
                       native_persistent_string const &message,
                       read::source_position const &start,
                       read::source_position const &end)
  {
    auto const file{ runtime::__rt_ctx->current_file_var->deref() };
    return runtime::make_box<error::base>(gc{},
                                          kind,
                                          message,
                                          read::source{ runtime::to_string(file), start, end });
  }
}
