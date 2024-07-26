#include <openssl/sha.h>

#include <jank/util/sha256.hpp>

namespace jank::util
{
  native_persistent_string sha256(native_persistent_string const &input)
  {
    std::array<unsigned char, SHA256_DIGEST_LENGTH> buf{};
    SHA256(reinterpret_cast<unsigned char const *>(input.c_str()), input.size(), buf.data());
    std::stringstream ss;
    for(auto const b : buf)
    {
      ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    return ss.str();
  }
}
