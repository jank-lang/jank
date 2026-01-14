#include <fstream>
#include <filesystem>

#include <clang/Format/Format.h>

#include <jank/util/clang_format.hpp>
#include <jank/util/environment.hpp>
#include <jank/util/fmt.hpp>

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

    std::filesystem::path const jank_path(process_dir().c_str());
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
        throw std::runtime_error{ util::format("unable to parse clang format yaml: {}",
                                               error_code.message()) };
      }
    }

    return *ret;
  }

  jtl::result<jtl::immutable_string, format_failure>
  format_cpp_source(jtl::immutable_string const &source)
  {
    std::string const code{ source };
    auto const &style(clang_format_style());
    auto replacements(reformat(style, code, llvm::ArrayRef(tooling::Range(0, code.size()))));
    auto formatted_code(tooling::applyAllReplacements(code, replacements));
    if(!formatted_code)
    {
      return err(llvm::toString(formatted_code.takeError()));
    }
    jtl::immutable_string const ret{ *formatted_code };
    return ok(ret);
  }
}
