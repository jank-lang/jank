with (import <nixpkgs> {});
mkShell
{
  buildInputs =
  [
    # Build deps.
    clang
    cling
    meson
    ninja
    boost

    # Dev tools.
    llvm
    entr
    ccls
    gcovr
    lcov
    git
  ];
  shellHook =
  ''
  export CLING_DEV="${lib.getDev pkgs.cling.unwrapped}"
  export CC="clang"
  export CXX="clang++"
  export CLING_INCLUDE_PATH="${pkgs.cling.unwrapped}/include"
  export CLING_LIB_PATH="${pkgs.cling.unwrapped}/lib"
  export LLVM_INCLUDE_PATH="${lib.getDev pkgs.llvmPackages_5.llvm}/include"
  export LLVM_ROOT_PATH="${pkgs.cling.unwrapped}"
  export BOOST_LIB_PATH="${pkgs.boost}/lib"

  function jank-configure
  {
    ${pkgs.coreutils}/bin/rm -rf build
    ${pkgs.meson}/bin/meson setup build -Dcling_include_path="''${CLING_INCLUDE_PATH}" \
                                        -Dcling_lib_path="''${CLING_LIB_PATH}" \
                                        -Dllvm_include_path="''${LLVM_INCLUDE_PATH}" \
                                        -Dllvm_root_path="''${LLVM_ROOT_PATH}" \
                                        "$@"
  }
  export -f jank-configure

  function jank-compile
  { ${pkgs.meson}/bin/meson compile -C build "$@"; }
  export -f jank-compile

  function jank-test
  { ./build/test/jank-unit-tests "$@"; }
  export -f jank-test

  function jank-watch-tests
  { ${pkgs.git}/bin/git ls-files -cdmo --exclude-standard | ${pkgs.entr}/bin/entr bash -c "./bin/run-unit-tests $@ || true"; }
  export -f jank-watch-tests

  function jank-install-deps
  {
    local install_dir="build-install"
    ${pkgs.coreutils}/bin/rm -rf "''${install_dir}"
    ${pkgs.coreutils}/bin/mkdir -p "''${install_dir}/include"
    ${pkgs.coreutils}/bin/cp -r lib/immer/immer  "''${install_dir}/include/"
    ${pkgs.coreutils}/bin/cp -r lib/magic_enum/include/*  "''${install_dir}/include/"
    ${pkgs.coreutils}/bin/cp -r ${lib.getDev pkgs.boost}/include/*  "''${install_dir}/include/"
    ${pkgs.coreutils}/bin/cp -r ${lib.getDev pkgs.llvmPackages_5.llvm}/include/*  "''${install_dir}/include/"
    ${pkgs.coreutils}/bin/chmod -R u+w "''${install_dir}/"
  }
  export -f jank-install-deps
  '';
}
