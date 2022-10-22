with (import <nixpkgs> {});
mkShell
{
  buildInputs =
  [
    # Build deps.
    cmake
    ninja
    cling
    llvmPackages_5.llvm

    # Dev tools.
    llvm # For clang-format
    entr
    gcovr
    lcov
    git
  ];
  shellHook =
  ''
  export CC="''${CC:-/usr/bin/clang}"
  export CXX="''${CXX:-/usr/bin/clang++}"

  export CLING_DEV="${lib.getDev pkgs.cling.unwrapped}"
  export LLVM_DEV="${lib.getDev pkgs.llvmPackages_5.llvm}"
  '';
}
