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
   * ```<syntax>
   * ```
   *
   * 1. item 1
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

  static void parse_text_span(jtl::immutable_string const &line, markdown_ref const md)
  {
    usize current_word{};
    bool last_word{};
    bool first_word{ true };
    jtl::string_builder span;
    text_style style;
    while(true)
    {
      auto const old_word{ current_word };
      auto const first_word_offset{ (first_word ? 0 : 1) };
      current_word = line.find(' ', current_word + first_word_offset);
      //util::println("old_word = {}, current_word = {}", old_word, current_word);
      if(current_word == jtl::immutable_string::npos)
      {
        last_word = true;
      }

      auto word{ line.substr(old_word + first_word_offset,
                             current_word - old_word - first_word_offset) };
      //util::println("word = '{}'", word);

      if(!first_word)
      {
        span(' ');
      }
      if(word.starts_with("**"))
      {
        if(!span.empty())
        {
          md->nodes.emplace_back(text_span{ span.release(), style });
          span.clear();
        }
        style.bold = !style.bold;
        word = word.substr(2);
      }
      if(word.starts_with("*"))
      {
        if(!span.empty())
        {
          md->nodes.emplace_back(text_span{ span.release(), style });
          span.clear();
        }
        style.italic = !style.italic;
        word = word.substr(1);
      }

      // **foo*bar**

      span(word);

      if(last_word)
      {
        break;
      }
      first_word = false;
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
    sb(' ');
  }

  void render_markdown(line_break const &, jtl::string_builder &sb, usize const)
  {
    sb('\n');
  }

  void render_markdown(inline_code const &code, jtl::string_builder &sb, usize const)
  {
    format_to(sb,
              "{}{}{} ",
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

**This is bold.** *This is italicized.*)") };
    util::println("{}", render_markdown(md));
  }
}
