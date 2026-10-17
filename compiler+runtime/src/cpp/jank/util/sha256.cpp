#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/SHA256.h>

#include <jank/util/sha256.hpp>

namespace jank::util
{
  jtl::immutable_string sha256(jtl::immutable_string const &input)
  {
    llvm::SHA256 hasher;
    hasher.update({ input.data(), input.size() });
    return llvm::toHex(hasher.final(), true);
  }
}
