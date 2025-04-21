#pragma once

#include <memory>
#include <map>

#include <jtl/primitive.hpp>

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
    std::map<usize, ftxui::Element>
    highlight(runtime::module::file_view const &code, usize line_start, usize line_end);
  }
}
