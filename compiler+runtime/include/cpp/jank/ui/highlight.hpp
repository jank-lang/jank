#pragma once

#include <jtl/primitive.hpp>
#include <jtl/immutable_string.hpp>

struct ic_highlight_env_s;
using ic_highlight_env_t = ic_highlight_env_s;

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

    void highlight_for_ic(ic_highlight_env_t * const henv, jtl::immutable_string_view const input);
  }
}
