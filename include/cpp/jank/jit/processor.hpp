#pragma once

#include <memory>

#include <cling/Interpreter/Interpreter.h>

#include <jank/result.hpp>
#include <jank/codegen/processor.hpp>

namespace jank::runtime
{ struct context; }

namespace jank::jit
{
  struct processor
  {
    processor(runtime::context &rt_ctx);

    result<option<runtime::object_ptr>, native_string> eval(codegen::processor &cg_prc) const;
    void eval_string(native_string const &s) const;

    std::unique_ptr<cling::Interpreter> interpreter;
  };
}
