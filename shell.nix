with (import <nixpkgs> {});
mkShell
{
  buildInputs =
  [
    # Build deps.
    cmake
    ninja
    libsodium
    liburing
    libunwind

    # Dev tools.
    llvm # For clang-format
    entr
    ccls
    gcovr
    lcov
    git
  ];
  shellHook =
  ''
  export CC="''${CC:-clang}"
  export CXX="''${CXX:-clang++}"
  '';
}
