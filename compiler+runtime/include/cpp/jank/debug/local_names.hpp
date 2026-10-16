#pragma once

#include <string>
#include <string_view>

#include <jtl/option.hpp>
#include <jtl/primitive.hpp>

namespace llvm
{
  class Module;
}

namespace jank::debug
{
  /* C++ can't spell locals like `foo-bar?`, so codegen declares them using a reserved
   * identifier which carries the raw jank name as hex. Hex has no underscores, so the
   * counter suffix (which keeps shadowed names unique) is unambiguous. Once Clang has
   * lowered those locals to LLVM debug metadata, `rewrite_local_names` swaps the encoded
   * identifier back to the raw name, so debuggers show what was written in jank. */
  std::string encode_local(std::string_view const &raw_name, usize const unique);
  /* Parameters are named in the generated signature, so the IR and codegen must agree on the
   * name. Without debug info, this is just the munged name. */
  std::string parameter_name(std::string_view const &raw_name, usize const index);
  jtl::option<std::string> decode_local(std::string_view const &encoded);

  /* Returns the number of variables renamed. */
  usize rewrite_local_names(llvm::Module &module);
}
