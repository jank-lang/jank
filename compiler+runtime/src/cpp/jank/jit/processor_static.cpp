#include <jank/jit/processor.hpp>
#include <jank/error/runtime.hpp>

namespace jank::jit
{
  processor::processor(jtl::immutable_string const &)
  {
  }

  processor::~processor() = default;

  void processor::eval_string(jtl::immutable_string const &) const
  {
    throw error::runtime_static_feature_disabled("eval");
  }

  runtime::object_ref processor::create_function(runtime::callable_arity_flags const,
                                                 jtl::immutable_string const &,
                                                 native_vector<u8> const &,
                                                 bool const) const
  {
    throw error::runtime_static_feature_disabled("eval");
  }
}
