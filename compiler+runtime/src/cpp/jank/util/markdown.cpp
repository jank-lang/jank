#include <variant>

#include <ftxui/screen/screen.hpp>

#include <jtl/format/style.hpp>

#include <jank/util/markdown.hpp>
#include <jank/util/string.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::util
{
  /* # h1
   * ## h2
   * ### h3
   * #### h4
   *
   * text_span
   *
   * `foo`
   *
   * **bold**
   *
   * *italic*
   *
   * terminal colors `redfoo`
   *
   * ```<syntax>
   * ```
   *
   * 1. item 1
   *   a. sub item
   * 2. item 2
   *
   * * item 1
   * * item 2
   *
   * BONUS
   *
   * > quote
   *
   * [foo](https://foo.com)
   */

  template <u8 size>
  struct h
  {
    jtl::immutable_string content;
  };

  using h1 = h<1>;
  using h2 = h<2>;
  using h3 = h<3>;
  using h4 = h<4>;
  static constexpr u8 header_max{ 4 };

  struct text_style
  {
    bool bold{};
    bool italic{};
  };

  struct text_span
  {
    jtl::immutable_string content;
    text_style style;
  };

  struct line_break
  {
  };

  struct inline_code
  {
    jtl::immutable_string content;
    text_style style;
  };

  using markdown_node = std::variant<h1, h2, h3, h4, text_span, line_break, inline_code>;

  struct markdown
  {
    native_vector<markdown_node> nodes;
  };

  static void parse_header(jtl::immutable_string const &line, markdown_ref const md)
  {
    u8 size{};
    usize i{};
    while(line[i] == '#')
    {
      ++i;
    }
    size = std::min(static_cast<u8>(i), header_max);

    auto const content{ trim(line.substr(i)) };
    switch(size)
    {
      case 1:
        md->nodes.emplace_back(h1{ content });
        break;
      case 2:
        md->nodes.emplace_back(h2{ content });
        break;
      case 3:
        md->nodes.emplace_back(h3{ content });
        break;
      case 4:
        md->nodes.emplace_back(h4{ content });
        break;
      default:
        jank_panic("Invalid markdown header.");
    }
  }

  static void parse_text_span(jtl::immutable_string line, markdown_ref const md)
  {
    jtl::string_builder span;
    text_style style;
    bool reading_inline_code{};
    while(!line.empty())
    {
      if(line.starts_with("**"))
      {
        if(!span.empty())
        {
          md->nodes.emplace_back(text_span{ span.release(), style });
          span.clear();
        }
        style.bold = !style.bold;
        line = line.substr(2);
        //println("toggled bold, line is now '{}'", line);
      }
      if(line.starts_with("*"))
      {
        if(!span.empty())
        {
          md->nodes.emplace_back(text_span{ span.release(), style });
          span.clear();
        }
        style.italic = !style.italic;
        line = line.substr(1);
        //println("toggled italics, line is now '{}'", line);
      }
      if(line.starts_with("`"))
      {
        if(reading_inline_code)
        {
          md->nodes.emplace_back(inline_code{ span.release(), style });
          span.clear();
          reading_inline_code = false;
        }
        else if(!span.empty())
        {
          md->nodes.emplace_back(text_span{ span.release(), style });
          span.clear();
          reading_inline_code = true;
        }
        line = line.substr(1);
      }

      span(line[0]);

      if(1 < line.size())
      {
        line = line.substr(1);
      }
      else
      {
        line = "";
      }
    }

    /* TODO: Helper for this? */
    if(!span.empty())
    {
      md->nodes.emplace_back(text_span{ span.release(), style });
    }
  }

  markdown_ref parse_markdown(jtl::immutable_string const &input)
  {
    auto const md{ jtl::make_ref<markdown>() };

    usize current_newline{};
    bool last_line{};
    bool first_line{ true };
    while(true)
    {
      auto const old_newline{ current_newline };
      auto const first_line_offset{ (first_line ? 0 : 1) };
      current_newline = input.find('\n', current_newline + first_line_offset);
      //util::println("old_newline = {}, current_newline = {}", old_newline, current_newline);
      if(current_newline == jtl::immutable_string::npos)
      {
        last_line = true;
      }

      auto const line{ input.substr(old_newline + first_line_offset,
                                    current_newline - old_newline - first_line_offset) };
      //util::println("line = '{}'", line);

      if(line.empty())
      {
        md->nodes.emplace_back(line_break{});
        md->nodes.emplace_back(line_break{});
      }
      else if(line.starts_with("#"))
      {
        parse_header(line, md);
      }
      else
      {
        parse_text_span(line, md);
      }

      if(last_line)
      {
        break;
      }
      first_line = false;
      //std::this_thread::sleep_for(std::chrono::milliseconds{ 1000 });
    }

    return md;
  }

  template <u8 Size>
  void render_markdown(h<Size> const &h, jtl::string_builder &sb, usize const max_width)
  {
    jtl::immutable_string padding_str;

    switch(Size)
    {
      case 1:
        padding_str = "━";
        break;
      case 2:
        padding_str = "─";
        break;
      case 3:
        padding_str = "─";
        break;
      case 4:
        padding_str = "-";
        break;
    }

    auto const padding_count{ max_width - 3 - h.content.size() };
    std::string padding;
    for(usize i{}; i < padding_count; ++i)
    {
      padding.insert(padding.size(), padding_str);
    }
    format_to(sb, "{} {} {}\n", padding_str, h.content, padding);
  }

  void render_markdown(text_span const &span, jtl::string_builder &sb, usize const)
  {
    if(span.style.bold)
    {
      sb(jtl::terminal_style::bold);
    }
    if(span.style.italic)
    {
      sb(jtl::terminal_style::italic);
    }
    sb(span.content);
    if(span.style.bold)
    {
      sb(jtl::terminal_style::no_bold);
    }
    if(span.style.italic)
    {
      sb(jtl::terminal_style::no_italic);
    }
  }

  void render_markdown(line_break const &, jtl::string_builder &sb, usize const)
  {
    sb('\n');
  }

  void render_markdown(inline_code const &code, jtl::string_builder &sb, usize const)
  {
    if(code.style.bold)
    {
      sb(jtl::terminal_style::bold);
    }
    if(code.style.italic)
    {
      sb(jtl::terminal_style::italic);
    }
    format_to(sb,
              "{}{}{}",
              jtl::terminal_style::bright_red,
              code.content,
              jtl::terminal_style::reset);
  }

  jtl::immutable_string render_markdown(markdown_ref const md)
  {
    jtl::string_builder sb;

    auto const terminal_width{ ftxui::Terminal::Size().dimx };
    auto const max_width{ std::min(terminal_width, 100) };

    for(auto const &node : md->nodes)
    {
      std::visit([&](auto const &typed_node) { render_markdown(typed_node, sb, max_width); }, node);
    }

    return sb.release();
  }

  void test_markdown()
  {
    auto const md{ parse_markdown(R"(# one
This is  a   sentence.
This is also a sentence.

This is a new  paragraph.

## Now for some styling
**This is bold. *This is** italicized.*

Here is some `code`. Here is **some `bold code`.**)") };
    util::println("{}", render_markdown(md));
  }
}
