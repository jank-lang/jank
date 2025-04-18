#include <jank/read/source.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/obj/nil.hpp>

namespace jank::read
{
  source_position const source_position::unknown{ 0, 0, 0 };
  source const source::unknown{ no_source_path,
                                source_position::unknown,
                                source_position::unknown };

  source::source(source_position const &start)
    : source{ start, start }
  {
  }

  source::source(source_position const &start, source_position const &end)
    : start{ start }
    , end{ end }
    , macro_expansion{ runtime::jank_nil }
  {
    auto const file{ runtime::__rt_ctx->current_file_var->deref() };
    file_path = runtime::to_string(file);
  }

  source::source(jtl::immutable_string const &file_path,
                 source_position const &start,
                 source_position const &end)
    : file_path{ file_path }
    , start{ start }
    , end{ end }
    , macro_expansion{ runtime::jank_nil }
  {
  }

  source::source(jtl::immutable_string const &file_path,
                 source_position const &start,
                 source_position const &end,
                 runtime::object_ref const macro_expansion)
    : file_path{ file_path }
    , start{ start }
    , end{ end }
    , macro_expansion{ macro_expansion }
  {
  }

  bool source_position::operator==(source_position const &rhs) const
  {
    return offset == rhs.offset && line == rhs.line && col == rhs.col;
  }

  bool source_position::operator!=(source_position const &rhs) const
  {
    return !(*this == rhs);
  }

  bool source_position::operator<=(source_position const &rhs) const
  {
    return offset <= rhs.offset;
  }

  bool source_position::operator>=(source_position const &rhs) const
  {
    return offset >= rhs.offset;
  }

  jtl::immutable_string source_position::to_string() const
  {
    util::string_builder sb;
    return sb("source_position(")(offset)(", ")(line)(":")(col)(")").release();
  }

  bool source::operator==(source const &rhs) const
  {
    return file_path == rhs.file_path && start == rhs.start && end == rhs.end;
  }

  bool source::operator!=(source const &rhs) const
  {
    return !(*this == rhs);
  }

  bool source::overlaps(source const &rhs) const
  {
    if(file_path != rhs.file_path)
    {
      return false;
    }
    return (rhs.start >= start && rhs.start <= end) || (rhs.end >= start && rhs.end <= end);
  }

  jtl::immutable_string source::to_string() const
  {
    util::string_builder sb;
    return sb("source(")(file_path)(" ")(start.to_string())(" -> ")(end.to_string())(")").release();
  }

  std::ostream &operator<<(std::ostream &os, source_position const &p)
  {
    return os << p.to_string();
  }

  std::ostream &operator<<(std::ostream &os, source const &s)
  {
    return os << s.to_string();
  }
}
