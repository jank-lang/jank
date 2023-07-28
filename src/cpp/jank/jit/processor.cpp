#include <cstdlib>

#include <cling/Interpreter/Value.h>
#include <clang/AST/Type.h>

#include <jank/util/process_location.hpp>
#include <jank/util/make_array.hpp>
#include <jank/jit/processor.hpp>

namespace jank::jit
{
  option<boost::filesystem::path> find_pch()
  {
    auto const jank_path(jank::util::process_location().unwrap().parent_path());

    auto dev_path(jank_path / "CMakeFiles/jank_lib.dir/cmake_pch.hxx.pch");
    if(boost::filesystem::exists(dev_path))
    { return std::move(dev_path); }

    auto installed_path(jank_path / "../include/cpp/jank/prelude.hpp.pch");
    if(boost::filesystem::exists(installed_path))
    { return std::move(installed_path); }

    return none;
  }

  option<boost::filesystem::path> build_pch()
  {
    auto const jank_path(jank::util::process_location().unwrap().parent_path());
    auto const script_path(jank_path / "build-pch");
    auto const include_path(jank_path / "../include");
    auto const command
    (script_path.string() + " " + include_path.string() + " " + std::string{ JANK_COMPILER_FLAGS });

    std::cerr << "Note: Looks like your first run. Building pre-compiled header… " << std::flush;

    if(std::system(command.c_str()) != 0)
    {
      std::cerr << "failed to build using this script: " << script_path << std::endl;
      return none;
    }

    std::cerr << "done!" << std::endl;
    return jank_path / "../include/cpp/jank/prelude.hpp.pch";
  }

  option<boost::filesystem::path> find_llvm_resource_path()
  {
    auto const jank_path(jank::util::process_location().unwrap().parent_path());

    if(boost::filesystem::exists(jank_path / "../lib/clang"))
    { return jank_path / ".."; }

    return JANK_CLING_BUILD_DIR;
  }

  processor::processor()
  {
    /* TODO: Pass this into each fn below so we only do this once on startup. */
    auto const jank_path(jank::util::process_location().unwrap().parent_path());

    auto pch_path(find_pch());
    if(pch_path.is_none())
    {
      pch_path = build_pch();

      /* TODO: Better error handling. */
      if(pch_path.is_none())
      { throw std::runtime_error{ "unable to find and also unable to build PCH" }; }
    }
    auto const &pch_path_str(pch_path.unwrap().string());

    auto const llvm_resource_path(find_llvm_resource_path());
    if(llvm_resource_path.is_none())
    /* TODO: Better error handling. */
    { throw std::runtime_error{ "unable to find LLVM resource path" }; }
    auto const &llvm_resource_path_str(llvm_resource_path.unwrap().string());

    auto const include_path(jank_path / "../include");

    auto const args
    (
      jank::util::make_array
      (
        "clang++", "-std=c++17",
        "-DHAVE_CXX14=1", "-DIMMER_HAS_LIBGC=1",
        "-include-pch", pch_path_str.c_str(),
        "-isystem", include_path.c_str(),

        "-O2", "-ffast-math", "-march=native"
      )
    );
    interpreter = std::make_unique<cling::Interpreter>(args.size(), args.data(), llvm_resource_path_str.c_str());

    /* TODO: Optimization >0 doesn't work with the latest Cling LLVM 13.
     * 1. https://github.com/root-project/cling/issues/483
     * 2. https://github.com/root-project/cling/issues/484
     */
  }

  result<option<runtime::object_ptr>, native_string> processor::eval
  (runtime::context &, codegen::processor &cg_prc) const
  {
    /* TODO: Improve Cling to accept string_views instead. */
    interpreter->declare(static_cast<std::string>(cg_prc.declaration_str()));

    auto const expr(cg_prc.expression_str(true, false));
    if(expr.empty())
    { return ok(none); }

    cling::Value v;
    /* TODO: Format this for erasure during codegen. */
    auto const result(interpreter->evaluate(fmt::format("&{}->base", expr), v));
    if(result != cling::Interpreter::CompilationResult::kSuccess)
    { return err("compilation error"); }

    // clang::QualType::getFromOpaquePtr(v.m_Type).getAsString()
    auto const ret_val(v.castAs<runtime::object*>());
    return ok(ret_val);
  }

  void processor::eval_string(native_string const &s) const
  { interpreter->process(static_cast<std::string>(s)); }
}
