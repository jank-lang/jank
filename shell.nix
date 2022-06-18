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
  export LLVM_PATH="${lib.getDev pkgs.llvmPackages_5.llvm}"
  export BOOST_LIB_PATH="${pkgs.boost}/lib"

  function jank-configure
  {
    ${pkgs.coreutils}/bin/rm -rf build
    ${pkgs.meson}/bin/meson setup build -Dcling_include_path="''${CLING_INCLUDE_PATH}" \
                                        -Dcling_lib_path="''${CLING_LIB_PATH}" \
                                        -Dllvm_path="''${LLVM_PATH}" \
                                        -Dboost_lib_path="''${BOOST_LIB_PATH}"
  }

  function jank-watch-unit-tests
  { ${pkgs.git}/bin/git ls-files -cdmo --exclude-standard | ${pkgs.entr}/bin/entr bash -c "./bin/run-unit-tests || true"; }

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
  '';
}
