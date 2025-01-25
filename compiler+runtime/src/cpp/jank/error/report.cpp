#include <algorithm>
#include <iostream>
#include <deque>

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

  static constexpr size_t max_body_lines{ 6 };
  static constexpr size_t min_body_lines{ 1 };
  static constexpr size_t max_top_margin_lines{ 2 };
  static constexpr size_t new_note_leniency_lines{ 2 };
  static constexpr size_t min_ellipsis_range{ 4 };

  struct line
  {
    enum class kind : uint8_t
    {
      file_data,
      note,
      ellipsis
    };

    kind kind{};
    /* Zero means no number. */
    size_t number{};
    /* Only set when kind == note. */
    option<note> note;
  };

  struct snippet
  {
    native_bool can_fit(note const &n) const;
    void add(read::source const &body_source, note const &n);
    void add_ellipsis(read::source const &body_source, note const &n);
    void add(note const &n);

    native_persistent_string file_path;
    /* Zero means we have no lines yet. */
    size_t line_start{};
    size_t line_end{};
    std::deque<line> lines{};
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

  native_bool snippet::can_fit(note const &n) const
  {
    assert(n.source.file_path == file_path);

    if(line_end == 0)
    {
      return true;
    }

    native_bool ret{ true };

    /* See if it can fit within our existing line coverage. */
    if(n.source.start.line < line_start)
    {
      ret &= line_start - n.source.start.line <= new_note_leniency_lines;
    }
    if(n.source.end.line > line_end)
    {
      ret &= n.source.end.line - line_end <= new_note_leniency_lines;
    }

    /* If it can, check each line to see if we have that line represented.
     * It could be that we have an ellipsis which covers the relevant lines. */
    if(ret)
    {
      ret = false;
      for(auto const &l : lines)
      {
        if(l.number == n.source.start.line)
        {
          ret = true;
          break;
        }
      }
    }

    return ret;
  }

  void snippet::add(note const &n)
  {
    add(n.source, n);
  }

  /* TODO: Make sure notes are sorted left to right. Handle multiple notes on one line. */
  void snippet::add(read::source const &body_source, note const &n)
  {
    assert(n.source.file_path == file_path);

    if(!can_fit(n))
    {
      add_ellipsis(body_source, n);
      return;
    }

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
      lines.emplace(lines.begin() + static_cast<ptrdiff_t>(i) + 1, line::kind::note, 0, n);
      break;
    }
  }

  void snippet::add_ellipsis(read::source const &body_source, note const &n)
  {
    /* First we add the note to an empty snippet. */
    snippet s{ .file_path = n.source.file_path };
    s.add(body_source, n);

    /* Then we stitch that snippet into this one, with an ellipsis in between.
     * There are three cases here, for where to put the new snippet:
     *
     * 1. At the start of our current snippet
     * 2. At the end of our current snippet
     * 3. In the middle of our current snippet
     */
    if(s.line_start < line_start)
    {
      lines.emplace(lines.begin(), line::kind::ellipsis);
      for(auto it{ s.lines.rbegin() }; it != s.lines.rend(); ++it)
      {
        lines.emplace(lines.begin(), std::move(*it));
      }
    }
    else if(line_end < s.line_start)
    {
      lines.emplace_back(line::kind::ellipsis);
      for(auto &line : s.lines)
      {
        lines.emplace_back(std::move(line));
      }
    }
    else
    {
      /* Inserting into the middle is messy. We only do this when the line where our note lives
       * is collapsed by an ellipsis. Thus, we need to expand the ellipsis either partially
       * or fully.
       *
       * We do this by finding the releveant ellipsis (there may be multiple) and inserting
       * our lines before it. We also insert our own ellipsis at the start of our lines. By
       * the end of this, we'll have our new lines with an ellipsis on either side. The
       * pass we run at the end to remove unneeded ellipsis will clean those up. */
      size_t last_ellipse{}, last_line_number{}, line_number_before_last_ellipse{};
      for(size_t i{}; i < lines.size(); ++i)
      {
        /* First loop until we find a line number larger than our start. We keep track
         * of the lines and ellipsis and we see them. */
        if(lines[i].kind == line::kind::file_data)
        {
          last_line_number = lines[i].number;
        }
        if(lines[i].kind == line::kind::ellipsis)
        {
          last_ellipse = i;
          line_number_before_last_ellipse = last_line_number;
        }
        else if(s.line_start < lines[i].number)
        {
          /* Once we've found a number which is beyond the start of our note, we
           * want to look back and find the last ellipsis. We can start inserting *before*
           * that ellipsis. */
          size_t added_lines{};
          for(size_t i{}; i < s.lines.size(); ++i)
          {
            /* Skip any duplicate lines which can mess with the ordering. */
            if(s.lines[i].number != 0 && s.lines[i].number <= line_number_before_last_ellipse)
            {
              continue;
            }
            lines.emplace(lines.begin() + static_cast<ptrdiff_t>(last_ellipse + added_lines),
                          std::move(s.lines[i]));
            ++added_lines;
          }
          /* We're inserting before the last ellipsis, so we end by putting an ellipsis at the
           * start of everything we just added. That surrounds our added lines in two ellipsis. */
          lines.emplace(lines.begin() + static_cast<ptrdiff_t>(last_ellipse), line::kind::ellipsis);

          break;
        }
      }
    }

    size_t last_number{};
    for(size_t i{}; i < lines.size(); ++i)
    {
      /* Remove ellipsis if needed. */
      if(lines[i].kind == line::kind::ellipsis)
      {
        /* We can be confident there's no note right after an ellipsis. */
        auto const diff(lines[i + 1].number - last_number);
        if(diff < min_ellipsis_range)
        {
          /* Fill in the extra lines. */
          lines[i].kind = line::kind::file_data;
          lines[i].number = last_number + 1;
          for(size_t k{ i + 1 }; k < i + diff - 1; ++k)
          {
            lines.emplace(lines.begin() + static_cast<ptrdiff_t>(k),
                          line::kind::file_data,
                          last_number + (k - i) + 1);
          }
        }
      }
      /* Ensure no overlap. */
      else if(last_number != 0 && lines[i].number == last_number)
      {
        lines.erase(lines.begin() + static_cast<ptrdiff_t>(i));
        --i;
      }

      if(lines[i].number != 0)
      {
        last_number = lines[i].number;
      }
    }
    static_cast<void>(last_number);
    static_cast<void>(min_ellipsis_range);

    line_start = std::min(line_start, s.line_start);
    line_end = std::max(line_end, s.line_end);
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
      if(snippet.file_path == n.source.file_path)
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
    /* TODO: Handle files in JARs. */
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
          line_content = highlighted_lines.at(l.number);
          break;
        case line::kind::note:
          line_number = text(" ");
          line_content = underline_note(l.note.unwrap());
          break;
        case line::kind::ellipsis:
          line_number = text(" ");
          line_content = text("…");
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
