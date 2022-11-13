#pragma once

#include <memory>

#include <cling/Interpreter/Interpreter.h>

#include <folly/FBString.h>

#include <jank/result.hpp>
#include <jank/runtime/context.hpp>
#include <jank/codegen/processor.hpp>

namespace jank::jit
{
  struct processor
  {
    processor();

    result<option<runtime::object_ptr>, folly::fbstring> eval
    (runtime::context &rt_ctx, codegen::processor &cg_prc) const;
    void eval_string(std::string const &s) const;

    std::unique_ptr<cling::Interpreter> interpreter;
  };
}
