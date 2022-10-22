#pragma once

#include <memory>

#include <folly/FBString.h>

#include <jank/result.hpp>
#include <jank/runtime/context.hpp>
#include <jank/codegen/processor.hpp>

namespace cling
{ class Interpreter; }

namespace jank::jit
{
  struct processor
  {
    processor();

    result<runtime::object_ptr, folly::fbstring> eval(runtime::context &rt_ctx, codegen::processor &cg_prc) const;

    std::unique_ptr<cling::Interpreter> interpreter;
  };
}
