#include <jank/error.hpp>

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
    /* TODO: File name. */
    return runtime::make_box<error::base>(gc{}, kind, message, read::source{ "", {}, {} });
  }

  error_ptr make_error(error::kind const kind,
                       native_persistent_string const &message,
                       read::source_position const &start)
  {
    /* TODO: File name. */
    /* NOLINTNEXTLINE(cppcoreguidelines-slicing) */
    return runtime::make_box<error::base>(gc{}, kind, message, read::source{ "", start, start });
  }

  error_ptr make_error(error::kind const kind,
                       native_persistent_string const &message,
                       read::source_position const &start,
                       read::source_position const &end)
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-slicing) */
    return runtime::make_box<error::base>(gc{}, kind, message, read::source{ "", start, end });
  }
}
