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
    processor();

    runtime::object_ptr eval(runtime::context &rt_ctx, codegen::processor &cg_prc) const;

    std::unique_ptr<cling::Interpreter> interpreter;
  };
}
