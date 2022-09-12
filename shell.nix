with (import <nixpkgs> {});
let
  # Nix has Conan 1.49.0, but Boost won't come in without 1.51.0, due to the following error:
  #   cannot import name 'is_apple_os' from 'conan.tools.apple'
  # We get around this by just copy/pasting the Nix expr for this, but upgrading as needed.
  conanPython = python310.override {
    packageOverrides = self: super: {
      node-semver = super.node-semver.overridePythonAttrs (oldAttrs: rec {
        version = "0.6.1";
        src = oldAttrs.src.override {
          inherit version;
          sha256 = "1dv6mjsm67l1razcgmq66riqmsb36wns17mnipqr610v0z0zf5j0";
        };
      });
      distro = super.distro.overridePythonAttrs (oldAttrs: rec {
        version = "1.5.0";
        src = oldAttrs.src.override {
          inherit version;
          sha256 = "14nz51cqlnxmgfqqilxyvjwwa5xfivdvlm0d0b1qzgcgwdm7an0f";
        };
      });
    };
  };
  conan = conanPython.pkgs.buildPythonApplication rec {
    version = "1.52.0";
    pname = "conan";

    src = fetchFromGitHub {
      owner = "conan-io";
      repo = "conan";
      rev = version;
      hash = "sha256-Y6J4vzbLxKPEOTLrS24n78iNjR6q5ri7F+f9jH+L8MU="; # 1.52.0
    };

    propagatedBuildInputs = with conanPython.pkgs; [
      bottle
      colorama
      python-dateutil
      deprecation
      distro
      fasteners
      future
      jinja2
      node-semver
      patch-ng
      pluginbase
      pygments
      pyjwt
      pylint
      pyyaml
      requests
      six
      tqdm
      urllib3
    ] ++ lib.optionals stdenv.isDarwin [ idna cryptography pyopenssl ];

    checkInputs = [
      pkg-config
      git
    ] ++ (with conanPython.pkgs; [
      codecov
      mock
      nose
      parameterized
      webtest
    ]);

  doCheck = false;

  postPatch = ''
  substituteInPlace conans/requirements_server.txt --replace 'PyJWT>=2.4.0, <3.0.0' 'PyJWT>=2.3.0'
  '';

  meta = with lib; {
    homepage = "https://conan.io";
    description = "Decentralized and portable C/C++ package manager";
    license = licenses.mit;
    maintainers = with maintainers; [ HaoZeke ];
  };
};
in
mkShell
{
  buildInputs =
  [
    # Build deps.
    #clang
    cling
    meson
    ninja
    folly
    boost
    conan

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
  export PKG_CONFIG_PATH="''${PWD}/build"

  function jank-configure
  {
    ${pkgs.coreutils}/bin/rm -rf build
    mkdir build
    #cd build ; conan install --build='folly' .. ; cd ..
    ${pkgs.meson}/bin/meson setup build --native-file vcpkg-meson.ini \
                                        -Dcling_include_path="''${CLING_INCLUDE_PATH}" \
                                        -Dcling_lib_path="''${CLING_LIB_PATH}" \
                                        -Dllvm_include_path="''${LLVM_INCLUDE_PATH}" \
                                        -Dllvm_root_path="''${LLVM_ROOT_PATH}" \
                                        "$@"
  }
  export -f jank-configure

  function jank-conan-build
  {
    cd build ; conan build .. ; cd ..
  }
  export -f jank-configure

  function jank-compile
  { ${pkgs.meson}/bin/meson compile -C build "$@"; }
  export -f jank-compile

  function jank-test
  { ./build/test/cpp/jank-unit-tests "$@"; }
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

  function jank-clean
  { /bin/rm -r build; }
  export -f jank-clean
  '';
}
