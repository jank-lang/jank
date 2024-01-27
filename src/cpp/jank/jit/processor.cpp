#include <cstdlib>

#include <cling/Interpreter/Value.h>
#include <clang/AST/Type.h>
//#include <Interpreter/IncrementalExecutor.h>
//#include <llvm/Support/MemoryBuffer.h>

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
    {
      return std::move(dev_path);
    }

    auto dev_arm64_path(jank_path / "CMakeFiles/jank_lib.dir/cmake_pch_arm64.hxx.pch");
    if(boost::filesystem::exists(dev_arm64_path))
    {
      return std::move(dev_arm64_path);
    }

    auto installed_path(jank_path / "../include/cpp/jank/prelude.hpp.pch");
    if(boost::filesystem::exists(installed_path))
    {
      return std::move(installed_path);
    }

    return none;
  }

  option<boost::filesystem::path> build_pch()
  {
    auto const jank_path(jank::util::process_location().unwrap().parent_path());
    auto const script_path(jank_path / "build-pch");
    auto const include_path(jank_path / "../include");
    auto const command(script_path.string() + " " + include_path.string() + " "
                       + std::string{ JANK_COMPILER_FLAGS });

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
    {
      return jank_path / "..";
    }

    return JANK_CLING_BUILD_DIR;
  }

  processor::processor(runtime::context &rt_ctx, native_integer const optimization_level)
    : optimization_level{ optimization_level }
  {
    profile::timer timer{ "jit ctor" };
    /* TODO: Pass this into each fn below so we only do this once on startup. */
    auto const jank_path(jank::util::process_location().unwrap().parent_path());

    auto pch_path(find_pch());
    if(pch_path.is_none())
    {
      pch_path = build_pch();

      /* TODO: Better error handling. */
      if(pch_path.is_none())
      {
        throw std::runtime_error{ "unable to find and also unable to build PCH" };
      }
    }
    auto const &pch_path_str(pch_path.unwrap().string());

    auto const llvm_resource_path(find_llvm_resource_path());
    if(llvm_resource_path.is_none())
    /* TODO: Better error handling. */
    {
      throw std::runtime_error{ "unable to find LLVM resource path" };
    }
    auto const &llvm_resource_path_str(llvm_resource_path.unwrap().string());

    auto const include_path(jank_path / "../include");

    native_persistent_string_view O{ "0" };
    switch(optimization_level)
    {
      case 0:
        break;
      case 1:
        O = "1";
        break;
      case 2:
        O = "2";
        break;
      case 3:
        O = "fast";
        break;
      default:
        throw std::runtime_error{ fmt::format("invalid optimization level {}",
                                              optimization_level) };
    }

    auto const args(jank::util::make_array(
      /* TODO: Path to clang++ from Cling build? Is this using the system clang++? */
      "clang++",
      "-std=c++17",
      "-DHAVE_CXX14=1",
      "-DIMMER_HAS_LIBGC=1",
      "-include-pch",
      pch_path_str.c_str(),
      "-isystem",
      include_path.c_str(),
      O.data()));
    interpreter = std::make_unique<cling::Interpreter>(args.size(),
                                                       args.data(),
                                                       llvm_resource_path_str.c_str());

    eval_string(fmt::format("auto &__rt_ctx(*reinterpret_cast<jank::runtime::context*>({}));",
                            fmt::ptr(&rt_ctx)));
  }

  result<option<runtime::object_ptr>, native_persistent_string>
  processor::eval(codegen::processor &cg_prc) const
  {
    profile::timer timer{ "jit eval" };
    /* TODO: Improve Cling to accept string_views instead. */
    auto const str(cg_prc.declaration_str());
    //fmt::println("{}", str);

    interpreter->declare(static_cast<std::string>(cg_prc.declaration_str()));

    auto const expr(cg_prc.expression_str(true));
    if(expr.empty())
    {
      return ok(none);
    }

    cling::Value v;
    /* TODO: Format this for erasure during codegen. */
    auto const result(interpreter->evaluate(fmt::format("&{}->base", expr), v));
    if(result != cling::Interpreter::CompilationResult::kSuccess)
    {
      return err("compilation error");
    }

    // clang::QualType::getFromOpaquePtr(v.m_Type).getAsString()
    auto const ret_val(v.castAs<runtime::object *>());
    return ok(ret_val);
  }

  void processor::eval_string(native_persistent_string const &s) const
  {
    jank::profile::timer timer{ "jit eval_string" };
    //fmt::println("JIT eval string {}", s);
    interpreter->process(static_cast<std::string>(s));
  }

  //void processor::load_object(native_persistent_string_view const &path) const
  //{
  //  auto buf(std::move(llvm::MemoryBuffer::getFile(path.data()).get()));
  //  llvm::cantFail(interpreter->m_Executor->m_JIT->Jit->addObjectFile(std::move(buf)));
  //  auto sym(interpreter->m_Executor->m_JIT->Jit->lookup("wow1"));
  //  if(auto e = sym.takeError())
  //  {
  //    fmt::println("sym error: {}", toString(std::move(e)));
  //    return;
  //  }
  //  reinterpret_cast<void (*)()>(sym->getAddress())();
  //}
}
