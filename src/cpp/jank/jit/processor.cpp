#include <cling/Interpreter/Value.h>

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

    auto installed_path(jank_path / "../include/cmake_pch.hxx.pch");
    if(boost::filesystem::exists(installed_path))
    { return std::move(installed_path); }

    return none;
  }

  processor::processor()
  {
    auto const pch_path(find_pch());
    if(pch_path.is_none())
    /* TODO: Better error handling. */
    { throw std::runtime_error{ "unable to find PCH path for JIT" }; }
    auto const &pch_path_str(pch_path.unwrap().string());

    auto const args
    (
      jank::util::make_array
      (
        "clang++", "-std=c++17",
        "-DHAVE_CXX14=1",
        "-include-pch", pch_path_str.c_str()
      )
    );
    interpreter = std::make_unique<cling::Interpreter>(args.size(), args.data(), LLVMDIR);
  }

  result<option<runtime::object_ptr>, std::string>  processor::eval
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
