#pragma once

#include <memory>

//#include <cling/Interpreter/Interpreter.h>
#include <clang/Interpreter/Interpreter.h>

#include <jank/result.hpp>
#include <jank/codegen/processor.hpp>

namespace jank::runtime
{
  struct context;
}

namespace jank::jit
{
  struct processor
  {
    processor(runtime::context &rt_ctx, native_integer optimization_level);
    ~processor();

    result<option<runtime::object_ptr>, native_persistent_string>
    eval(codegen::processor &cg_prc) const;
    void eval_string(native_persistent_string const &s) const;
    void load_object(native_persistent_string_view const &path) const;

    //std::unique_ptr<cling::Interpreter> interpreter;
    std::unique_ptr<clang::Interpreter> interpreter;
    native_integer optimization_level{};
  };
}
