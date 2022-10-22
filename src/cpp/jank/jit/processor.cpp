#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#pragma clang diagnostic pop

#include <jank/util/process_location.hpp>
#include <jank/util/make_array.hpp>
#include <jank/jit/processor.hpp>

namespace jank::jit
{
  processor::processor()
  {
    /* TODO: Store initial state during install and load it on each use. */
    auto const jank_location(jank::util::process_location().unwrap().parent_path());
    auto const args(jank::util::make_array("clang++", "-std=c++17"));
    interpreter = std::make_unique<cling::Interpreter>(args.size(), args.data(), LLVMDIR);

    interpreter->AddIncludePath(jank_location.string() + "/../include");
    interpreter->AddIncludePath(jank_location.string() + "/../include/cpp");
    /* TODO: Figure out how to make this easier for dev. */
    interpreter->AddIncludePath(jank_location.string() + "/vcpkg_installed/x64-linux/include");
    interpreter->AddIncludePath(jank_location.string() + "/vcpkg_installed/x64-osx/include");

    /* TODO: Pre-compiled prelude. */
    interpreter->loadHeader("jank/prelude.hpp");
  }

  runtime::object_ptr processor::eval(runtime::context &, codegen::processor &cg_prc) const
  {
    interpreter->process(cg_prc.declaration_str());
    cling::Value v;
    interpreter->evaluate(cg_prc.expression_str(), v);
    return *v.simplisticCastAs<runtime::object_ptr*>();
  }
}
