#pragma once

#include <memory>

#include <jank/runtime/context.hpp>
#include <jank/codegen/processor.hpp>

namespace cling
{ class Interpreter; }

namespace jank::jit
{
  struct processor
  {
    processor() = delete;
    processor(runtime::context &rt_ctx);

    void eval(codegen::processor const &cg_prc);

    runtime::context &rt_ctx;
    std::unique_ptr<cling::Interpreter> interpreter;
  };
}
