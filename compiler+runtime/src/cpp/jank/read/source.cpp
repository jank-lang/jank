#include <jank/read/source.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/to_string.hpp>

namespace jank::read
{
  source_position source_position::unknown()
  {
    return { 0, 0, 0 };
  }

  source source::unknown()
  {
    return { no_source_path,
             no_source_path,
             source_position::unknown(),
             source_position::unknown() };
  }

  source::source(source_position const &start)
    : source{ start, start }
  {
  }

  source::source(source_position const &start, source_position const &end)
    : file{ runtime::to_string(runtime::__rt_ctx->current_file_var->deref()) }
    , module{ runtime::to_code_string(runtime::__rt_ctx->current_ns_var->deref()) }
    , start{ start }
    , end{ end }
  {
  }

  source::source(jtl::immutable_string const &file,
                 jtl::immutable_string const &module,
                 source_position const &start,
                 source_position const &end)
    : file{ file }
    , module{ module }
    , start{ start }
    , end{ end }
  {
  }

  source::source(jtl::immutable_string const &file,
                 jtl::immutable_string const &module,
                 source_position const &start,
                 source_position const &end,
                 runtime::object_ref const macro_expansion)
    : file{ file }
    , module{ module }
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
    jtl::string_builder sb;
    return sb("source_position(")(offset)(", ")(line)(":")(col)(")").release();
  }

  bool source::operator==(source const &rhs) const
  {
    /* TODO: Can this be default? Missing macro_expansion right now. */
    return file == rhs.file && module == rhs.module && start == rhs.start && end == rhs.end;
  }

  bool source::operator!=(source const &rhs) const
  {
    return !(*this == rhs);
  }

  bool source::overlaps(source const &rhs) const
  {
    if(file != rhs.file || module != rhs.module)
    {
      return false;
    }
    return (rhs.start >= start && rhs.start <= end) || (rhs.end >= start && rhs.end <= end);
  }

  jtl::immutable_string source::to_string() const
  {
    jtl::string_builder sb;
    return sb("source(")(file)(" (")(module)(") ")(start.to_string())(" -> ")(end.to_string())(")")
      .release();
  }

  std::ostream &operator<<(std::ostream &os, source_position const &p)
  {
    return os << p.to_string().c_str();
  }

  std::ostream &operator<<(std::ostream &os, source const &s)
  {
    return os << s.to_string().c_str();
  }
}
