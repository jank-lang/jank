#include <fstream>

#include <fmt/format.h>

#include <clang/Format/Format.h>

#include <jank/util/clang_format.hpp>
#include <jank/util/process_location.hpp>
#include <jank/util/dir.hpp>

namespace jank::util
{
  using namespace clang;
  using namespace clang::format;

  static std::string const &clang_format_yaml()
  {
    static std::string ret;
    if(!ret.empty())
    {
      return ret;
    }

    auto const jank_path(process_location().unwrap().parent_path());
    auto path(jank_path / ".clang-format");

    if(!std::filesystem::exists(path))
    {
      path = jank_path / "../share/.clang-format";
      if(!std::filesystem::exists(path))
      {
        throw std::runtime_error{ "unable to find .clang-format" };
      }
    }

    std::ifstream ifs{ path.string() };
    ret = std::string{ std::istreambuf_iterator<char>{ ifs }, {} };

    return ret;
  }

  static FormatStyle const &clang_format_style()
  {
    static FormatStyle *ret{};
    if(!ret)
    {
      ret = new FormatStyle{ getNoStyle() };
      auto const error_code(parseConfiguration(clang_format_yaml(), ret, true));
      if(error_code)
      {
        delete ret;
        ret = nullptr;
        throw std::runtime_error{ fmt::format("unable to parse clang format yaml: {}",
                                              error_code.message()) };
      }
    }

    return *ret;
  }

  result<native_persistent_string, format_failure>
  format_cpp_source(native_persistent_string const &source)
  {
    std::string const code{ source };
    auto const &style(clang_format_style());
    auto replacements(reformat(style, code, llvm::ArrayRef(tooling::Range(0, code.size()))));
    auto formatted_code(tooling::applyAllReplacements(code, replacements));
    if(!formatted_code)
    {
      return err(llvm::toString(formatted_code.takeError()));
    }
    return ok(native_persistent_string{ *formatted_code });
  }
}
