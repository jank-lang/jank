#include <jank/runtime/context.hpp>
#include <jank/error/runtime.hpp>

namespace jank::runtime
{
  object_ref context::eval(object_ref const o)
  {
    //throw error::runtime_static_feature_disabled("eval");
    return o;
  }

  jtl::result<void, error_ref> context::compile_module(jtl::immutable_string const &)
  {
    throw error::runtime_static_feature_disabled("compile");
  }

  object_ref context::read_file(jtl::immutable_string const &, object_ref const)
  {
    throw error::runtime_static_feature_disabled("read_file");
  }
}
