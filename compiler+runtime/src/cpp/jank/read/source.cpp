#include <jank/read/source.hpp>

namespace jank::read
{
  static constexpr auto pos_max{ std::numeric_limits<size_t>::max() };
  source_position const source_position::unknown{ pos_max, pos_max, pos_max };
  source const source::unknown{ "unknown", source_position::unknown, source_position::unknown };

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
