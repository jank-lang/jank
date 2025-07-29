#include <filesystem>

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/string.hpp>

#include <jank/environment/check_health.hpp>
#include <jank/util/process_location.hpp>
#include <jank/util/clang.hpp>
#include <jank/util/dir.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::environment
{
  using namespace ftxui;

  /* Each status report must start with one of:
   *
   * - ✅ for expected behavior
   * - ⚠️ for warnings
   * - ❌ for errors
   *
   * Colors for these should be green, yellow, and red. All paths should be blue. */

  static Element jank_version()
  {
    return vbox({
      hbox({
        text("─ ✅ ") | color(Color::Green),
        text("jank version: "),
        text(JANK_VERSION),
      }),
    });
  }

  static Element jank_resource_dir()
  {
    std::filesystem::path dir{ JANK_RESOURCE_DIR };
    bool relative{};
    auto const jank_path{ util::process_location().unwrap().parent_path() };
    if(!dir.is_absolute())
    {
      dir = (jank_path / dir);
      relative = true;
    }
    auto const exists{ std::filesystem::exists(dir) };
    auto const dev_build{ jank_path.filename() == "build"
                          && jank_path.parent_path().filename() == "compiler+runtime" };
    auto const icon{ exists ? "✅"
                            /* NOLINTNEXTLINE(readability-avoid-nested-conditional-operator) */
                            : (dev_build ? "✅" : "❌") };
    /* NOLINTNEXTLINE(readability-avoid-nested-conditional-operator) */
    auto const col{ exists ? Color::Green : (dev_build ? Color::Yellow : Color::Red) };

    Elements res{ hbox({
      text(util::format("─ {} ", icon)) | color(col),
      text("jank resource dir: "),
      text(JANK_RESOURCE_DIR) | color(Color::Blue),
      text(exists ? " (found)" : " (not found)") | color(Color::GrayDark),
    }) };

    if(relative)
    {
      res.emplace_back(hbox({
        text(util::format("─ {} ", icon)) | color(col),
        text(util::format("jank resolved resource dir: ")),
        text(dir.c_str()) | color(Color::Blue),
        text(dev_build ? " (ignored for dev build)" : " (not found)") | color(Color::GrayDark),
      }));
    }

    return vbox(res);
  }

  static Element jank_user_cache_dir()
  {
    auto const path{ util::user_cache_dir(util::binary_version()) };
    auto const configured_path_exists{ std::filesystem::exists(path.c_str()) };
    return hbox({
      text("─ ✅ ") | color(configured_path_exists ? Color::Green : Color::Yellow),
      text("jank user cache dir: "),
      text(path.c_str()) | color(Color::Blue),
      text(util::format("{}", configured_path_exists ? " (found)" : " (not found)"))
        | color(Color::GrayDark),
    });
  }

  static Element clang_path()
  {
    auto const configured_path_exists{ std::filesystem::exists(JANK_CLANG_PATH) };
    Elements res{
      hbox({
        text(util::format("─ {} ", configured_path_exists ? "✅" : "⚠️ "))
          | color(configured_path_exists ? Color::Green : Color::Yellow),
        text("configured clang path: "),
        text(JANK_CLANG_PATH) | color(Color::Blue),
        text(util::format("{}", configured_path_exists ? " (found)" : " (not found)"))
          | color(Color::GrayDark),
      }),
    };

    auto const found_clang{ util::find_clang() };
    auto const found_path_exists{ found_clang.is_some()
                                    ? std::filesystem::exists(found_clang.unwrap().c_str())
                                    : false };
    if(found_path_exists && found_clang.unwrap() != JANK_CLANG_PATH)
    {
      res.emplace_back(hbox({
        text("─ ✅ ") | color(Color::Green),
        text("runtime clang path: "),
        text(found_clang.unwrap()) | color(Color::Blue),
        text(" (found)") | color(Color::GrayDark),
      }));
    }
    else if(!found_path_exists)
    {
      res.emplace_back(hbox({
        text("─ ❌ ") | color(Color::Red),
        text(util::format("clang version {} not found in configured location or on PATH",
                          JANK_CLANG_MAJOR_VERSION))
          | color(Color::Red),
      }));
    }

    return vbox(res);
  }

  static Element clang_resource_root()
  {
    /* TODO: Runtime resource root. */
    auto const configured_path_exists{ std::filesystem::exists(JANK_CLANG_RESOURCE_DIR) };
    return hbox({
      text(util::format("─ {} ", configured_path_exists ? "✅" : "⚠️ "))
        | color(configured_path_exists ? Color::Green : Color::Yellow),
      text("configured clang resource dir: "),
      text(JANK_CLANG_RESOURCE_DIR) | color(Color::Blue),
      text(util::format("{}", configured_path_exists ? " (found)" : " (not found)"))
        | color(Color::GrayDark),
    });
  }

  /* TODO: Run health check before runtime context is created, so we don't try to make
   * PCH shit. */
  static Element pch_location()
  {
    auto const pch_path{ util::find_pch(util::binary_version()) };
    if(pch_path.is_some())
    {
      return hbox({
        text("─ ✅ ") | color(Color::Green),
        text("jank pch path: "),
        text(pch_path.unwrap()) | color(Color::Blue),
        text(" (found)") | color(Color::GrayDark),
      });
    }

    return hbox({
      text("─ ✅ ") | color(Color::Green),
      text("jank pch dir: "),
      text(util::user_cache_dir(util::binary_version())) | color(Color::Blue),
      text(" (no pch found, should be built automatically)") | color(Color::GrayDark),
    });
  }

  static Element header(std::string const &title, usize const max_width)
  {
    auto const padding_count(max_width - 3 - title.size());
    std::string padding;
    for(usize i{}; i < padding_count; ++i)
    {
      padding.insert(padding.size(), "─");
    }
    return hbox({
      text("─ ") | color(Color::GrayDark),
      text(title) | color(Color::BlueLight),
      text(" "),
      text(padding) | color(Color::GrayDark),
    });
  }

  /* Runs through the various jank systems and outputs to stdout various status reports.
   * This is meant to be useful for debugging distribution/installation issues.
   *
   * Returns whether or not jank is healthy. */
  bool check_health()
  {
    auto const terminal_width{ Terminal::Size().dimx };
    auto const max_width{ std::min(terminal_width, 100) };

    std::vector<Element> const doc_body{
      header("jank install", max_width),
      jank_version(),
      jank_resource_dir(),
      jank_user_cache_dir(),
      text(" "),

      header("clang install", max_width),
      clang_path(),
      clang_resource_root(),
      text(" "),
      header("jank runtime", max_width),
      pch_location(),
    };

    auto document{ vbox(doc_body) };
    auto screen{ Screen::Create(Dimension::Full(), Dimension::Fit(document)) };
    Render(screen, document);
    screen.Print();
    util::print("\n");

    return true;
  }
}
