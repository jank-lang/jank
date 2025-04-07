#pragma once

#include <memory>
#include <map>

namespace ftxui
{
  using Element = std::shared_ptr<struct Node>;
}

namespace jtl
{
  struct immutable_string;
}

namespace jank
{

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
