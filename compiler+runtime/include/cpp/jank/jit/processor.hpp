#pragma once

#include <memory>

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
    processor(util::cli::options const &opts);
    ~processor();

    result<option<runtime::object_ptr>, native_persistent_string>
    eval(codegen::processor &cg_prc) const;
    void eval_string(native_persistent_string const &s) const;
    void load_object(native_persistent_string const &path) const;

    result<void, native_persistent_string>
    load_dynamic_libs(native_vector<native_persistent_string> const &libs) const;

    option<native_persistent_string> find_dynamic_lib(native_persistent_string const &lib) const;

    std::unique_ptr<clang::Interpreter> interpreter;
    native_integer optimization_level{};
    native_vector<boost::filesystem::path> library_dirs;
  };
}
