#pragma once

#include <jtl/primitive.hpp>
#include <jtl/immutable_string.hpp>

namespace jank
{
  namespace runtime::module
  {
    struct file_view;
  }

  namespace ui
  {
    native_map<usize, jtl::immutable_string>
    highlight(runtime::module::file_view const &code, usize line_start, usize line_end);

    jtl::immutable_string highlight_str(runtime::module::file_view const &code);
    jtl::immutable_string
    highlight_str(runtime::module::file_view const &code, usize line_start, usize line_end);
  }
}
