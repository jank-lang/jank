#include <cling/Interpreter/Value.h>

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

  result<option<runtime::object_ptr>, folly::fbstring>  processor::eval
  (runtime::context &, codegen::processor &cg_prc) const
  {
    interpreter->process(cg_prc.declaration_str());

    auto const expr(cg_prc.expression_str());
    if(expr.empty())
    { return ok(none); }

    /* TODO: Check if the memory for his value needs to be released. */
    cling::Value v;
    auto const result(interpreter->evaluate(expr, v));
    if(result != cling::Interpreter::CompilationResult::kSuccess)
    { return err("compilation error"); }

    auto const *ret_val(v.simplisticCastAs<runtime::object_ptr*>());
    return ok(*ret_val);

    /* TODO: Just return the fn to be called once the Cling bug is fixed. */

    //auto const ret_val(v.simplisticCastAs<runtime::object*>());
    //if(ret_val == nullptr)
    //{ return err("JIT evaluation returned nullptr"); }
    //std::cout << ret_val->to_string() << std::endl;
    //auto const callable(ret_val->as_callable());
    //if(callable == nullptr)
    //{ return err("Returned JIT object is not callable"); }
    //return ok(some(callable->call()));
  }

  //void processor::eval_string(std::string const &s) const
  //{
  //  cling::Value v;
  //  auto const result(interpreter->evaluate(s, v));
  //  if(result != cling::Interpreter::CompilationResult::kSuccess)
  //  { return; }
  //}
}
