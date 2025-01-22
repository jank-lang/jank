#pragma once

#include <memory>
#include <vector>

namespace ftxui
{
  using Element = std::shared_ptr<struct Node>;
}

namespace jank
{
  struct native_persistent_string;

  namespace ui
  {
    std::vector<ftxui::Element>
    highlight(native_persistent_string const &code, size_t line_start, size_t line_end);
  }
}
