#include <algorithm>
#include <iostream>

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

  struct line
  {
    enum class kind : uint8_t
    {
      file_data,
      note,
    };

    kind kind{};
    /* Zero means no number. */
    size_t number{};
    /* Only for notes. */
    option<note> note;
  };

  /* TODO: Use an ... between distant notes. */
  struct snippet
  {
    static constexpr size_t max_body_lines{ 5 };
    static constexpr size_t min_body_lines{ 1 };
    static constexpr size_t max_top_margin_lines{ 2 };
    static constexpr size_t new_note_leniency_lines{ 2 };

    native_bool can_fit(note const &n) const;
    void add(read::source const &body_source, note const &n);
    void add(note const &n);

    native_persistent_string file_path;
    /* Zero means we have no lines yet. */
    size_t line_start{};
    size_t line_end{};
    /* TODO: Consider deque. */
    native_vector<line> lines;
  };

  struct plan
  {
    plan(error_ptr const e);

    void add(read::source const &body_source, note const &n);
    void add(note const &n);

    native_persistent_string kind;
    native_persistent_string message;
    native_vector<snippet> snippets;
  };

  plan::plan(error_ptr const e)
    : kind{ kind_str(e->kind) }
    , message{ e->message }
  {
    add(e->source, e->error_note);
    for(auto const &n : e->extra_notes)
    {
      add(n);
    }
  }

  static size_t absdiff(size_t const lhs, size_t const rhs)
  {
    if(lhs < rhs)
    {
      return rhs - lhs;
    }
    return lhs - rhs;
  }

  native_bool snippet::can_fit(note const &n) const
  {
    assert(n.source.file_path == file_path);

    if(line_end == 0)
    {
      return true;
    }

    native_bool ret{ true };

    if(n.source.start.line < line_start)
    {
      /* TODO: Don't need this fn, right? We know which is larger. */
      ret &= absdiff(line_start, n.source.start.line) <= new_note_leniency_lines;
    }
    if(n.source.end.line > line_end)
    {
      ret &= absdiff(line_end, n.source.end.line) <= new_note_leniency_lines;
    }

    /* TODO: Also can't fit if it's on the same line as another note. (exception being to the left of it?) */

    return ret;
  }

  void snippet::add(note const &n)
  {
    add(n.source, n);
  }

  void snippet::add(read::source const &body_source, note const &n)
  {
    assert(n.source.file_path == file_path);
    assert(can_fit(n));

    static constexpr size_t max_body_lines{ 6 };
    static constexpr size_t min_body_lines{ 1 };
    static constexpr size_t max_top_margin_lines{ 2 };

    /* Top margin:
     *   Min: 1 line
     *   Max: 2 lines
     *
     *   If the count goes negative, use one blank line.
     *
     * Code body:
     *   Min: 1 line
     *   Max: 5 lines
     *
     *   If the error spans more than the max, just show up to the max.
     *
     * Bottom margin:
     *   Always 0
     */

    if(line_start == 0)
    {
      auto const body_range{
        std::clamp(n.source.end.line - body_source.start.line, min_body_lines, max_body_lines) - 1
      };
      auto const top_margin{ std::min(body_source.start.line - 1, max_top_margin_lines) };
      line_end = n.source.end.line;
      line_start = line_end - body_range - top_margin;
      //fmt::println("source.start {} source.end {}", body_source.start.line, body_source.end.line);
      //fmt::println("body_range {} line_start {} line_end {}", body_range, line_start, line_end);

      for(size_t i{ line_start }; i <= line_end; ++i)
      {
        lines.emplace_back(line::kind::file_data, i);
      }
    }
    else
    {
      if(n.source.start.line < line_start)
      {
        for(auto i{ line_start - 1 }; i >= n.source.start.line; --i)
        {
          lines.emplace(lines.begin(), line::kind::file_data, i);
        }
        line_start = n.source.start.line;
      }
      if(n.source.end.line > line_end)
      {
        for(auto i{ line_end + 1 }; i <= n.source.end.line; ++i)
        {
          lines.emplace_back(line::kind::file_data, i);
        }
        line_end = n.source.end.line;
      }
    }

    //auto const line_index{ n.source.start.line - line_start };
    //lines.emplace(lines.begin() + line_index + 1, line::kind::note, 0, n);
    for(size_t i{}; i < lines.size(); ++i)
    {
      if(lines[i].number != n.source.start.line)
      {
        continue;
      }
      while(lines[i].kind == line::kind::note && i < lines.size())
      {
        ++i;
      }
      lines.emplace(lines.begin() + i + 1, line::kind::note, 0, n);
      break;
    }
  }

  void plan::add(note const &n)
  {
    add(n.source, n);
  }

  void plan::add(read::source const &body_source, note const &n)
  {
    native_bool added{ false };
    for(auto &snippet : snippets)
    {
      if(snippet.file_path == n.source.file_path && snippet.can_fit(n))
      {
        snippet.add(body_source, n);
        added = true;
        break;
      }
    }

    if(!added)
    {
      snippets.emplace_back(n.source.file_path).add(body_source, n);
    }
  }

  static Element underline_note(note const &n)
  {
    auto const width{ std::max(n.source.end.col - n.source.start.col, 1zu) };
    std::string line(n.source.start.col - 1, ' ');
    line.insert(line.end(), width, '^');
    line += ' ';
    line += n.message;

    auto const ret{ text(line) };
    switch(n.kind)
    {
      case note::kind::info:
        return ret | color(Color::BlueLight);
      case note::kind::warning:
        return ret | color(Color::Orange1);
      case note::kind::error:
        return ret | color(Color::Red);
    }
  }

  static Element code_snippet(snippet const &s)
  {
    /* TODO: Handle unknown source and failure to map. */
    auto const file(util::map_file(s.file_path));
    if(file.is_err())
    {
      /* TODO: Return result. */
      throw std::runtime_error{
        fmt::format("unable to map file {} due to error: {}", s.file_path, file.expect_err())
      };
    }

    auto const highlighted_lines{
      ui::highlight({ file.expect_ok().head, file.expect_ok().size }, s.line_start, s.line_end)
    };
    //fmt::println("highlighted_lines {}", highlighted_lines.size());

    std::vector<Element> line_numbers, lines;

    size_t highlighted_line_idx{};
    for(auto const &l : s.lines)
    {
      Element line_number{}, line_content{};
      //fmt::println("snippet line {} : {}",
      //             l.number,
      //             l.kind == line::kind::file_data ? "file" : "note");
      switch(l.kind)
      {
        case line::kind::file_data:
          line_number = paragraphAlignRight(std::to_string(l.number));
          line_content = highlighted_lines.at(highlighted_line_idx);
          ++highlighted_line_idx;
          break;
        case line::kind::note:
          line_number = text(" ");
          line_content = underline_note(l.note.unwrap());
          break;
      }
      line_numbers.emplace_back(line_number);
      lines.emplace_back(line_content);
    }

    /* TODO: line + col */
    return window(text(fmt::format(" {} ", s.file_path)),
                  hbox({ vbox(std::move(line_numbers)), separator(), vbox(std::move(lines)) }));
  }

  void report(error_ptr const e)
  {
    static constexpr size_t max_width{ 80 };

    plan p{ e };

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

    std::vector<Element> doc_body{
      error,
      text("\n"),
    };

    for(auto const &s : p.snippets)
    {
      doc_body.emplace_back(code_snippet(s));
    }

    auto document{ vbox(doc_body) | size(WIDTH, LESS_THAN, max_width) };
    auto screen{ Screen::Create(Dimension::Full(), Dimension::Fit(document)) };
    Render(screen, document);
    std::cout << screen.ToString() << '\0' << '\n';
  }
}
