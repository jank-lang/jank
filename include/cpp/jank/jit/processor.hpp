#pragma once

#include <memory>

#include <cling/Interpreter/Interpreter.h>

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
    processor(native_integer optimization_level);

    result<option<runtime::object_ptr>, native_persistent_string>
    eval(codegen::processor &cg_prc) const;
    void eval_string(native_persistent_string const &s) const;
    void load_object(native_persistent_string_view const &path) const;

    std::unique_ptr<cling::Interpreter> interpreter;
    native_integer optimization_level{};
  };
}
