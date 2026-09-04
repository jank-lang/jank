#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include <jank/jit/parse_ld_script.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::jit
{
  namespace
  {
    struct temporary_file
    {
      explicit temporary_file(std::string const &contents)
        : path{ std::filesystem::temp_directory_path()
                / ("jank-ld-script-test-"
                   + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())) }
      {
        std::ofstream file{ path, std::ios::binary };
        REQUIRE(file.good());
        file << contents;
      }

      ~temporary_file()
      {
        std::error_code error;
        std::filesystem::remove(path, error);
      }

      std::filesystem::path path;
    };

    jtl::immutable_string path_string(std::filesystem::path const &path)
    {
      return path.string();
    }
  }

  TEST_SUITE("jit ld script")
  {
    TEST_CASE("parses GROUP and selects the required library")
    {
      temporary_file const script{ "/* GNU ld script */\n"
                                   "OUTPUT_FORMAT(elf64-x86-64)\n"
                                   "GROUP ( /lib/libm.so.6 AS_NEEDED ( /lib/libmvec.so.1 ) )\n" };

      CHECK_FALSE(is_object_file(path_string(script.path)));
      auto const result{ parse_ld_script(path_string(script.path)) };
      REQUIRE(result.is_some());
      CHECK(result.unwrap() == "/lib/libm.so.6");
    }

    TEST_CASE("parses INPUT")
    {
      temporary_file const script{ "INPUT ( libexample.so )\n" };

      auto const result{ parse_ld_script(path_string(script.path)) };
      REQUIRE(result.is_some());
      CHECK(result.unwrap() == "libexample.so");
    }

    TEST_CASE("falls back to an AS_NEEDED library")
    {
      temporary_file const script{ "GROUP ( AS_NEEDED ( /lib/liboptional.so ) )\n" };

      auto const result{ parse_ld_script(path_string(script.path)) };
      REQUIRE(result.is_some());
      CHECK(result.unwrap() == "/lib/liboptional.so");
    }

    TEST_CASE("prefers a required library over an earlier AS_NEEDED library")
    {
      temporary_file const script{
        "GROUP ( AS_NEEDED ( /lib/liboptional.so ) /lib/librequired.so )\n"
      };

      auto const result{ parse_ld_script(path_string(script.path)) };
      REQUIRE(result.is_some());
      CHECK(result.unwrap() == "/lib/librequired.so");
    }

    TEST_CASE("recognizes object files")
    {
      temporary_file const object{ std::string{ "\x7f"
                                                "ELF"
                                                "binary contents" } };

      CHECK(is_object_file(path_string(object.path)));
      CHECK(parse_ld_script(path_string(object.path)).is_none());
    }
  }
}
