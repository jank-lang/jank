#pragma once

#include <memory>
#include <map>

namespace ftxui
{
  using Element = std::shared_ptr<struct Node>;
}

namespace jank
{
  struct native_persistent_string;

  namespace runtime::module
  {
    struct file_view;
  }

  namespace ui
  {
    std::map<size_t, ftxui::Element>
    highlight(runtime::module::file_view const &code, size_t line_start, size_t line_end);
  }
}
