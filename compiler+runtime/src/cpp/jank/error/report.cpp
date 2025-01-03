#include <iostream>
#include <algorithm>

#include <fmt/format.h>

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/string.hpp>

#include <jank/util/mapped_file.hpp>
#include <jank/error/report.hpp>
#include <jank/ui/highlight.hpp>
#include <jank/native_persistent_string/fmt.hpp>

namespace jank::error
{
  using namespace jank;
  using namespace jank::runtime;
  using namespace ftxui;

  Element snippet(error_ptr const e)
  {
    static constexpr size_t max_body_lines{ 5 };
    static constexpr size_t min_body_lines{ 1 };

    static constexpr size_t max_top_margin_lines{ 2 };

    /* Top margin:
     *   Min: 1 line
     *   Max: 2 lines
     *
     *   If the count goes negative, use one blank line.
     *
     * Bottom margin:
     *   Always 0
     *
     * Code body:
     *   Min: 1 line
     *   Max: 5 lines
     *
     *   If the error spans more than the max, just show up to the max.
     */

    auto const body_range{
      std::min(std::max(e->source.end.line - e->source.start.line, min_body_lines), max_body_lines)
    };
    auto const top_margin{ std::min(e->source.start.line - 1, max_top_margin_lines) };
    auto const line_start{ e->source.start.line - top_margin };
    auto const line_end{ e->source.start.line + body_range };

    std::vector<Element> line_numbers;
    for(auto i{ static_cast<ssize_t>(line_start) }; i < static_cast<ssize_t>(line_end); ++i)
    {
      if(i < 1)
      {
        line_numbers.emplace_back(text(" "));
      }
      else
      {
        line_numbers.emplace_back(text(std::to_string(i)));
      }
    }

    auto const file(util::map_file(e->source.file_path));
    if(file.is_err())
    {
      /* TODO: Return result. */
      throw std::runtime_error{ fmt::format("unable to map file {} due to error: {}",
                                            e->source.file_path,
                                            file.expect_err()) };
    }

    return window(
      text(
        fmt::format(" {}:{}:{} ", e->source.file_path, e->source.start.line, e->source.start.col)),
      hbox(
        { vbox(line_numbers) | color(Color::GrayLight),
          separator(),
          ui::highlight({ file.expect_ok().head, file.expect_ok().size }, line_start, line_end) }));
  }

  void report(error_ptr const e)
  {
    static constexpr size_t max_width{ 80 };

    auto header{ [&](std::string const &title) {
      auto const padding_count(max_width - 2 - title.size());
      std::string padding;
      for(size_t i{}; i < padding_count; ++i)
      {
        padding.insert(padding.size(), "─");
      }
      return hbox({
        text("─ "),
        text(title) | color(Color::BlueLight),
        text(" "),
        text(padding),
      });
    } };

    auto error{ vbox({ header(kind_str(e->kind)),
                       hbox({
                         text("error: ") | bold | color(Color::Red),
                         text(e->message) | bold,
                       }) }) };

    /* TODO: Context. */
    //auto context{ vbox(
    //  { header("context"),
    //    vbox({
    //      hbox({ text("compiling module "), text("kitten.main") | color(Color::Yellow) }),
    //      hbox({
    //        text("└─ requiring module "),
    //        text("kitten.nap") | color(Color::Yellow),

    //      }),
    //      hbox({
    //        text("   └─ compiling function "),
    //        text("kitten.nap/") | color(Color::Yellow),
    //        text("nap") | color(Color::Green),
    //      }),
    //    }) }) };

    auto document{ vbox({
      error,
      text("\n"),
      snippet(e),
      text("\n"),
      //context(),
    }) };

    document = document | size(WIDTH, LESS_THAN, max_width);

    auto screen{ Screen::Create(Dimension::Full(), Dimension::Fit(document)) };
    Render(screen, document);
    std::cout << screen.ToString() << '\0' << std::endl;
  }
}
