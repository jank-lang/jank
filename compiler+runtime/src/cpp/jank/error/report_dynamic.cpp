#include <algorithm>
#include <deque>

#include <jtl/terminal.hpp>

#include <jank/util/string.hpp>
#include <jank/util/fmt.hpp>
#include <jank/error/report.hpp>
#include <jank/ui/highlight.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/path.hpp>

namespace jank::error
{
  using jtl::terminal::text_style;
  using namespace jank;
  using namespace jank::runtime;

  static constexpr usize max_body_lines{ 6 };
  static constexpr usize min_body_lines{ 1 };
  static constexpr usize max_top_margin_lines{ 2 };
  static constexpr usize new_note_leniency_lines{ 2 };
  static constexpr usize min_ellipsis_range{ 4 };

  struct line
  {
    enum class kind : u8
    {
      file_data,
      note,
      ellipsis
    };

    static constexpr char const *kind_str(kind const k)
    {
      switch(k)
      {
        case kind::file_data:
          return "file_data";
        case kind::note:
          return "note";
        case kind::ellipsis:
          return "ellipsis";
      }
      return "unknown";
    }

    kind kind{};
    /* Zero means no number. */
    usize number{};
    /* Only set when kind == note. */
    jtl::option<note> note;
  };

  struct snippet
  {
    bool can_fit_without_ellipsis(note const &n) const;
    void add(read::source const &body_source, note const &n);
    void add_ellipsis(read::source const &body_source, note const &n);
    void add(note const &n);

    jtl::immutable_string module;
    jtl::immutable_string file;
    /* Zero means we have no lines yet. */
    usize line_start{};
    usize line_end{};
    std::deque<line> lines{};
  };

  struct plan
  {
    plan(error_ref const e);

    void add(read::source const &body_source, note const &n);
    void add(note const &n);

    jtl::immutable_string kind;
    jtl::immutable_string message;
    native_vector<snippet> snippets;
  };

  plan::plan(error_ref const e)
    : kind{ kind_str(e->kind) }
    , message{ e->message }
  {
    e->sort_notes();

    for(auto const &n : e->notes)
    {
      add(n);
    }
  }

  bool snippet::can_fit_without_ellipsis(note const &n) const
  {
    jank_debug_assert(n.source.module == module);

    if(line_end == 0)
    {
      return true;
    }

    bool ret{ true };

    /* See if it can fit within our existing line coverage. */
    if(n.source.start.line < line_start)
    {
      ret &= (line_start - n.source.start.line) <= new_note_leniency_lines;
    }
    if(line_end < n.source.start.line)
    {
      ret &= (n.source.start.line - line_end) <= new_note_leniency_lines;
    }

    /* If it can, check each line to see if we have that line represented.
     * It could be that we have an ellipsis which covers the relevant lines.
     * In that case, it doesn't fit.
     *
     * Only do this if the note lives within the existin bounds. If we're
     * adding to the end, we don't need to look for an ellipsis. */
    if(ret && n.source.start.line < line_end)
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

  /* TODO: Handle multiple notes on one line by building bridges. */
  void snippet::add(read::source const &body_source, note const &n)
  {
    jank_debug_assert(n.source.module == module);

    /* Adding a note follows some rules to determine how it will fit.
     *
     * If it's the first note, we'll just make sure it has some top
     * margin. We'll also limit the body of the note, in case it spans too many lines.
     * We favor the bottom portion of the lines, rather than the top. We never add
     * bottom margin.
     *
     * If it's the second note, we need to determine if it's close enough
     * to the first note. We have some leniency for adding new lines if the
     * two are close, but the second note is just outside the visible lines.
     * However, if it's well beyond that, we create an ellipsis which fills
     * the gaps between those lines.
     *
     * If it's the third note or beyond, we do the same steps as the second, but
     * we also need to check if the note we're placing falls within an ellipsis
     * gap. If so, we have the intricate logic of needing to determine if
     * we're at the top of the ellipsis, at the bottom, or in the middle. The
     * outcome here may result in our note with an ellipsis on top/bottom or
     * perhaps the ellipsis going away altogether.
     */

    if(!can_fit_without_ellipsis(n))
    {
      add_ellipsis(body_source, n);
      return;
    }

    if(line_start == 0)
    {
      auto const body_range{
        std::clamp(n.source.end.line - body_source.start.line, min_body_lines, max_body_lines) - 1
      };
      auto const top_margin{ std::min(body_source.start.line - 1, max_top_margin_lines) };
      line_end = n.source.end.line;
      line_start = line_end - body_range - top_margin;

      for(usize i{ line_start }; i <= line_end; ++i)
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

    for(usize i{}; i < lines.size(); ++i)
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
    snippet s{ .module = n.source.module, .file = n.source.file };
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
       * We do this by finding the releveant ellipses (there may be multiple) and inserting
       * our lines before it. We also insert our own ellipsis at the start of our lines. By
       * the end of this, we'll have our new lines with an ellipsis on either side. The
       * pass we run at the end to remove unneeded ellipsis will clean those up. */
      usize last_ellipsis{}, last_line_number{}, line_number_before_last_ellipsis{};
      for(usize i{}; i < lines.size(); ++i)
      {
        /* First loop until we find a line number larger than our start. We keep track
         * of the lines and ellipsis and we see them. */
        if(lines[i].kind == line::kind::file_data)
        {
          last_line_number = lines[i].number;
        }
        if(lines[i].kind == line::kind::ellipsis)
        {
          last_ellipsis = i;
          line_number_before_last_ellipsis = last_line_number;
        }
        else if(s.line_start < lines[i].number)
        {
          /* Once we've found a number which is beyond the start of our note, we
           * want to look back and find the last ellipsis. We can start inserting *before*
           * that ellipsis. */
          usize added_lines{};
          for(usize i{}; i < s.lines.size(); ++i)
          {
            /* Skip any duplicate lines which can mess with the ordering. */
            if(s.lines[i].number != 0 && s.lines[i].number <= line_number_before_last_ellipsis)
            {
              continue;
            }
            lines.emplace(lines.begin() + static_cast<ptrdiff_t>(last_ellipsis + added_lines),
                          std::move(s.lines[i]));
            ++added_lines;
          }
          /* We're inserting before the last ellipsis, so we end by putting an ellipsis at the
           * start of everything we just added. That surrounds our added lines in two ellipsis. */
          lines.emplace(lines.begin() + static_cast<ptrdiff_t>(last_ellipsis),
                        line::kind::ellipsis);

          break;
        }
      }
    }

    usize last_number{};
    for(usize i{}; i < lines.size(); ++i)
    {
      /* Remove ellipsis, if needed. */
      if(lines[i].kind == line::kind::ellipsis)
      {
        /* We can be confident there's no note right after an ellipsis, since
         * notes only follow normal lines or other notes. */
        auto const diff(lines[i + 1].number - last_number);
        if(diff < min_ellipsis_range)
        {
          /* Fill in the extra lines. */
          lines[i].kind = line::kind::file_data;
          lines[i].number = last_number + 1;
          for(usize k{ i + 1 }; k < i + diff - 1; ++k)
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

    line_start = std::min(line_start, s.line_start);
    line_end = std::max(line_end, s.line_end);
  }

  void plan::add(note const &n)
  {
    add(n.source, n);
  }

  void plan::add(read::source const &body_source, note const &n)
  {
    if(n.source == read::source::unknown())
    {
      return;
    }

    bool added{ false };
    for(auto &snippet : snippets)
    {
      if(snippet.file == n.source.file)
      {
        snippet.add(body_source, n);
        added = true;
        break;
      }
    }

    if(!added)
    {
      snippets.emplace_back(n.source.module, n.source.file).add(body_source, n);
    }
  }

  static jtl::immutable_string header(std::string const &title, usize const max_width)
  {
    auto const padding_count(max_width - 3 - title.size());
    jtl::string_builder sb;
    sb(text_style::bright_black);
    sb("─ ");
    sb(text_style::bright_blue);
    sb(title);
    sb(text_style::bright_black);
    sb(" ");
    for(usize i{}; i < padding_count; ++i)
    {
      sb("─");
    }
    sb(text_style::reset);
    return sb.release();
  }

  /* In order to flex properly, we need line number columns to all be the same width.
   * This requires some work, since the numbers themselves can vary. For example, a
   * snippet spanning lines 8 through 12 will have two lines (8 - 9) which have a
   * single character width, while (10 - 12) are twice as wide. So, we measure the
   * width of the widest number (the last one) and then pad the others. */
  static jtl::immutable_string line_number(usize const max_line_number_width, std::string num)
  {
    if(num.size() < max_line_number_width)
    {
      /* We add space to the beginning so we can keep numbers right-aligned. */
      num.insert(num.begin(), max_line_number_width - num.size(), ' ');
    }

    constexpr auto const target_width{ 5 };
    auto const left_padding{ (target_width - num.size()) / 2 };
    auto const right_padding{ static_cast<u64>(
      std::ceil(static_cast<double>(target_width - num.size()) / 2.0)) };

    jtl::string_builder sb;
    sb(text_style::bright_black);
    for(u64 i{}; i < left_padding; ++i)
    {
      sb(' ');
    }
    sb(num);
    for(u64 i{}; i < right_padding; ++i)
    {
      sb(' ');
    }
    sb(text_style::reset);
    return sb.release();
  }

  /* TODO: Support Unicode as well. */
  /* Counts the length of a string, ignoring ANSI escape sequences. */
  static usize display_length(jtl::immutable_string const &line)
  {
    usize length{};
    for(usize i{ 0 }; i < line.size() - 2; ++i)
    {
      if(line[i] == '\u001b')
      {
        i += 3;
        if(line[i] != 'm')
        {
          ++i;
        }
        continue;
      }

      ++length;
    }

    return length;
  }

  static jtl::immutable_string
  underline_note(note const &n, jtl::immutable_string const &previous_line_content)
  {
    jtl::string_builder sb;

    switch(n.kind)
    {
      case note::kind::info:
      case note::kind::info_line:
        sb(text_style::bright_blue);
      case note::kind::warning:
      case note::kind::warning_line:
        sb(text_style::yellow);
      case note::kind::error:
      case note::kind::error_line:
        sb(text_style::red);
    }

    if(note::kind::line_start <= n.kind && n.kind <= note::kind::line_end)
    {
      usize line_start{};
      for(; line_start < previous_line_content.size(); ++line_start)
      {
        if(previous_line_content[line_start] == '\u001b')
        {
          line_start += 3;
          if(previous_line_content[line_start] == 'm')
          {
            ++line_start;
          }
          else
          {
            line_start += 2;
          }
        }

        if(std::isspace(previous_line_content[line_start]))
        {
          sb(' ');
          continue;
        }
        break;
      }

      sb("╰");
      auto const length{ display_length(previous_line_content.substr(line_start)) };
      for(usize i{ 0 }; i < length - 2; ++i)
      {
        sb("─");
      }
      sb("╯ ");
      sb(n.message);
    }
    else
    {
      auto const width{ std::max(n.source.end.col - n.source.start.col, 1llu) };
      std::string underline(n.source.start.col - 1, ' ');
      underline.insert(underline.end(), width, '^');
      underline += ' ';

      sb(underline);
      sb(n.message);
    }

    return sb.release();
  }

  static jtl::immutable_string
  code_snippet_box(std::filesystem::path const &path,
                   native_vector<jtl::immutable_string> const &line_numbers,
                   native_vector<jtl::immutable_string> const &line_contents,
                   usize const max_width)
  {
    static constexpr auto margin{ 8 };
    std::string top_line{ "─────┬──" };
    for(usize i{ margin }; i < max_width; ++i)
    {
      top_line += "─";
    }
    std::string const pre_title{ "     │ " };
    std::string middle_line{ "─────┼──" };
    for(usize i{ margin }; i < max_width; ++i)
    {
      middle_line += "─";
    }
    std::string bottom_line{ "─────┼──" };
    for(usize i{ margin }; i < max_width; ++i)
    {
      bottom_line += "─";
    }

    jtl::string_builder sb;
    util::format_to(sb, "{}{}\n", text_style::bright_black, top_line);
    util::format_to(sb,
                    "{}{}{}{}{}\n",
                    pre_title,
                    text_style::reset,
                    text_style::bold,
                    util::compact_path(path, max_width - margin),
                    text_style::reset);
    util::format_to(sb, "{}{}\n", text_style::bright_black, middle_line);
    for(usize i{}; i < line_numbers.size(); ++i)
    {
      util::format_to(sb,
                      "{}{}│{} {}\n",
                      line_numbers[i],
                      text_style::bright_black,
                      text_style::reset,
                      line_contents[i]);
    }
    util::format_to(sb, "{}{}{}", text_style::bright_black, bottom_line, text_style::reset);
    return sb.release();
  }

  static jtl::immutable_string code_snippet(snippet const &s, usize const max_width)
  {
    /* We may not be able to read the file, so we fall back to trying to read the module.
     * This can happen for baked-in core libs like clojure.core for an installed jank. */
    auto file(runtime::module::loader::read_file(s.file));
    if(file.is_err())
    {
      if(s.file != read::no_source_path)
      {
        file = runtime::__rt_ctx->module_loader.read_module(s.module);
      }
      if(file.is_err())
      {
        return util::format("Unable to read file: {}\n{}\n", s.file, file.expect_err()->message);
      }
    }

    auto const highlighted_lines{ ui::highlight(file.expect_ok(), s.line_start, s.line_end) };

    native_vector<jtl::immutable_string> line_numbers, lines;
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    auto const max_line_number_width{ snprintf(nullptr, 0, "%llu", s.line_end) };

    jtl::immutable_string line_num{}, line_content{}, previous_line_content{};
    for(auto const &l : s.lines)
    {
      switch(l.kind)
      {
        case line::kind::file_data:
          line_num = line_number(max_line_number_width, std::to_string(l.number));
          /* TODO:There's a bug here. We should always contain this line number. */
          if(highlighted_lines.contains(l.number))
          {
            line_content = highlighted_lines.at(l.number);
          }
          else
          {
            line_content = " ";
          }
          previous_line_content = line_content;
          break;
        case line::kind::note:
          line_num = line_number(max_line_number_width, "");
          line_content = underline_note(l.note.unwrap(), previous_line_content);
          break;
        case line::kind::ellipsis:
          line_num = line_number(max_line_number_width, "");
          line_content = "…";
          break;
      }
      line_numbers.emplace_back(line_num);
      lines.emplace_back(line_content);
    }

    /* We want to render paths as relative, if we can. This not only keeps them shorter,
     * it also makes our test suite more portable. */
    return code_snippet_box(util::relative_path(file.expect_ok().file_path()),
                            line_numbers,
                            lines,
                            max_width);
  }

  static jtl::immutable_string documentation_box(error_ref const e, usize const max_width)
  {
    static constexpr auto margin{ 8 };
    std::string const pre_title{ "  🗎  │ " };
    std::string bottom_line{ "─────┴──" };
    for(usize i{ margin }; i < max_width; ++i)
    {
      bottom_line += "─";
    }

    jtl::string_builder sb;
    util::format_to(sb,
                    "{}{}{}https://book.jank-lang.org/reference/error/{}.html{}\n",
                    text_style::reset,
                    text_style::bright_black,
                    pre_title,
                    kind_str(e->kind),
                    text_style::reset);
    util::format_to(sb, "{}{}{}\n", text_style::bright_black, bottom_line, text_style::reset);
    return sb.release();
  }

  void report(error_ref const e)
  {
    plan const p{ e };

    auto const terminal_width{ jtl::terminal::get_size().width };
    auto const max_width{ std::min(terminal_width, 100ull) };

    util::println("{}", header(kind_str(e->kind), max_width));
    util::println("{}error:{} {}\n",
                  text_style::bold | text_style::red,
                  text_style::reset,
                  e->message);

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

    for(auto const &s : p.snippets)
    {
      util::println("{}", code_snippet(s, max_width));
    }
    if(p.snippets.empty())
    {
      util::println("https://book.jank-lang.org/reference/error/{}.html\n", kind_str(e->kind));
    }
    else
    {
      util::println("{}\n", documentation_box(e, max_width));
    }

    if(e->cause)
    {
      report(e->cause.as_ref());
    }
  }
}
