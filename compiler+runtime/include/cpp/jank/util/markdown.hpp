#pragma once

#include <jtl/ref.hpp>

namespace jank::util
{
  using markdown_ref = jtl::ref<struct markdown>;

  markdown_ref parse_markdown(jtl::immutable_string const &input);
  jtl::immutable_string render_markdown(markdown_ref const md);

  void test_markdown();
}
