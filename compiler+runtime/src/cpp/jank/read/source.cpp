#include <jank/read/source.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/to_string.hpp>

namespace jank::read
{
  source_position const source_position::unknown{ 0, 0, 0 };
  source const source::unknown{ "NO_SOURCE_PATH",
                                source_position::unknown,
                                source_position::unknown };

  source::source(source_position const &start)
    : source{ start, start }
  {
  }

  source::source(source_position const &start, source_position const &end)
    : start{ start }
    , end{ end }
  {
    auto const file{ runtime::__rt_ctx->current_file_var->deref() };
    file_path = runtime::to_string(file);
  }

  source::source(native_persistent_string const &file_path,
                 source_position const &start,
                 source_position const &end)
    : file_path{ file_path }
    , start{ start }
    , end{ end }
  {
  }

  native_bool source_position::operator==(source_position const &rhs) const
  {
    return offset == rhs.offset && line == rhs.line && col == rhs.col;
  }

  native_bool source_position::operator!=(source_position const &rhs) const
  {
    return !(*this == rhs);
  }

  native_bool source::operator==(source const &rhs) const
  {
    return file_path == rhs.file_path && start == rhs.start && end == rhs.end;
  }

  native_bool source::operator!=(source const &rhs) const
  {
    return !(*this == rhs);
  }

  std::ostream &operator<<(std::ostream &os, source_position const &p)
  {
    return os << "source_position(" << p.offset << ", " << p.line << ":" << p.col << ")";
  }

  std::ostream &operator<<(std::ostream &os, source const &s)
  {
    return os << "source(" << s.file_path << " " << s.start << " -> " << s.end << ")";
  }
}
