#pragma once

#include <memory>

namespace ftxui
{
  using Element = std::shared_ptr<struct Node>;
}

namespace jank
{
  struct native_persistent_string;

  namespace ui
  {
    ftxui::Element
    highlight(native_persistent_string const &code, size_t const line_start, size_t const line_end);
  }
}
