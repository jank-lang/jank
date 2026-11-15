#include <jank/error/aot.hpp>
#include <jank/util/environment.hpp>
#include <jank/aot/target_config.hpp>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace jank::aot
{
  void from_json(nlohmann::json const &j, target_config &c)
  {
    j.at("triple").get_to(c.triple);
    j.at("clang_bin").get_to(c.clang_bin);
    j.at("jit_flags").get_to(c.jit_flags);
    j.at("aot_flags").get_to(c.aot_flags);
  }

  jtl::result<target_config, error_ref> load_target_config(std::string const &path)
  {
    std::string actual_path{ path };
    if(path.empty())
    {
      actual_path = std::filesystem::path{ util::process_dir() } / "target_config.json";
    }

    if(!std::filesystem::exists(actual_path))
    {
      return error::aot_internal_failure(
        util::format("target json file does not exist: {}", actual_path));
    }

    std::ifstream ifs{ actual_path };
    auto json = nlohmann::json::parse(ifs, nullptr, false);
    if(json.is_discarded())
    {
      return error::aot_internal_failure(
        util::format("invalid target json syntax: {}", actual_path));
    }

    return json.get<target_config>();
  }
}
