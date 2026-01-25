{
  description = "Dev environment for jank";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    self.submodules = true;
  };

  outputs = inputs @ {flake-parts, ...}:
    flake-parts.lib.mkFlake {inherit inputs;} {
      systems = ["x86_64-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin"];
      perSystem = {
        self',
        pkgs,
        lib,
        ...
      }: let
        llvmPackages = pkgs.llvmPackages_22;
        # for cpptrace; versions from cpptrace/cmake/OptionVariables.cmake
        libdwarf-lite-src = pkgs.fetchFromGitHub {
          owner = "jeremy-rifkin";
          repo = "libdwarf-lite";
          rev = "5e71a74491dddc231664bbcd6a8cf8a8643918e9";
          sha256 = "sha256-qHikjAG5xuuHquqqKGuiDHXVZSlg/MbNp9JNSAKM/Hs=";
        };
        zstd-src = pkgs.fetchFromGitHub {
          owner = "facebook";
          repo = "zstd";
          rev = "v1.5.7";
          sha256 = "sha256-tNFWIT9ydfozB8dWcmTMuZLCQmQudTFJIkSr0aG7S44=";
        };
        # Manually set compilation and linker flags, rather than depending on
        # them to be implicitly set in the clang wrapper scripts. This is so
        # that the jank build process can pick up the flags such that they can
        # be passed along to downstream jank AOT compilation commands.
        cmakeCxxFlags = lib.concatStringsSep " " [
          (lib.trim (lib.readFile "${llvmPackages.clang}/nix-support/cc-cflags"))
          (lib.trim (lib.readFile "${llvmPackages.clang}/nix-support/libc-crt1-cflags"))
        ];
        cmakeLinkerFlags = lib.concatStringsSep " " [
          (lib.trim (lib.readFile "${llvmPackages.clang}/nix-support/cc-ldflags"))
          "-Wl,-rpath,${llvmPackages.stdenv.cc.libc}/lib"
          "-L${lib.getLib llvmPackages.libllvm.lib}/lib"
          "-L${lib.getLib pkgs.bzip2}/lib"
          "-L${lib.getLib pkgs.openssl}/lib"
          "-L${lib.getLib pkgs.zlib}/lib"
          "-L${lib.getLib pkgs.zstd}/lib"
          "-L${lib.getLib pkgs.libedit}/lib"
          "-L${lib.getLib pkgs.libxml2}/lib"
        ];
      in {
        legacyPackages = pkgs;
        formatter = pkgs.alejandra;

        packages = rec {
          default = jank-release;

          jank-release = llvmPackages.stdenv.mkDerivation (finalAttrs: {
            pname = "jank";
            version = "git";

            # Add only essential files so that the source hash is consistent.
            src = lib.cleanSource (lib.fileset.toSource {
              root = ./.;
              fileset = lib.fileset.unions [
                ./.clang-format
                ./compiler+runtime
              ];
            });

            nativeBuildInputs =
              [
                llvmPackages.clang
                llvmPackages.libclang.dev
              ]
              ++ (with pkgs; [
                cmake
                git
                ninja
              ]);

            buildInputs =
              [
                llvmPackages.libllvm.dev
              ]
              ++ (with pkgs; [
                bzip2
                openssl
                zstd
                libedit
                libxml2
              ]);

            checkInputs = with pkgs; [
              doctest
              glibcLocales
            ];

            postPatch = ''
              patchShebangs ./compiler+runtime/bin/ar-merge
            '';

            preConfigure = ''
              cmakeFlagsArray+=(
                "-DCMAKE_CXX_FLAGS=${lib.escapeShellArg cmakeCxxFlags}"
                "-DCMAKE_EXE_LINKER_FLAGS=${lib.escapeShellArg cmakeLinkerFlags}"
                "-DCMAKE_SHARED_LINKER_FLAGS=${lib.escapeShellArg cmakeLinkerFlags}"
                "-DCMAKE_MODULE_LINKER_FLAGS=${lib.escapeShellArg cmakeLinkerFlags}"
              )
            '';

            cmakeBuildDir = "./compiler+runtime/build";
            cmakeDir = "..";
            cmakeFlags = [
              # TODO: Updating RPATHs during install causes the step to fail as it
              # tries to rewrite non-existent RPATHs like /lib. Needs more
              # investigation.
              "-DCMAKE_SKIP_RPATH=ON"
              # Manually provide any FetchContent sources as network requests are
              # not allowed in the nix build sandbox.
              "-DFETCHCONTENT_SOURCE_DIR_LIBDWARF=${libdwarf-lite-src}"
              "-DFETCHCONTENT_SOURCE_DIR_ZSTD=${zstd-src}"
              # jank options
              (lib.cmakeBool "jank_unity_build" true)
              (lib.cmakeBool "jank_test" finalAttrs.doCheck)
              # We run out of memory in CI without this.
              (lib.cmakeBool "jank_force_phase_2" true)
            ];

            # Use a UTF-8 locale or else tests which use UTF-8 characters will
            # fail. See: https://github.com/NixOS/nixpkgs/issues/172752
            LC_ALL = "C.UTF-8";

            doCheck = true;
            checkPhase = ''
              pushd ../
              ./build/jank-test
              popd
            '';
          });
        };

        devShells.default = (pkgs.mkShell.override {stdenv = llvmPackages.stdenv;}) {
          packages = let
            cmake' = pkgs.writeShellScriptBin "cmake" ''
              exec ${pkgs.cmake}/bin/cmake -DCMAKE_CXX_FLAGS=${lib.escapeShellArg cmakeCxxFlags} "$@"
            '';
          in
            with pkgs; [
              stdenv.cc.cc.lib

              ## Required tools.
              cmake
              cmake'
              ninja
              pkg-config
              llvmPackages.libclang
              llvmPackages.libllvm

              ## Required libs.
              boehmgc
              openssl

              ## Dev tools.
              babashka
              entr
              gcovr
              lcov
              git
              nixd
              shellcheck
              # For clangd & clang-tidy.
              clang-tools
              gdb
              clangbuildanalyzer
              openjdk

              ## Dev libs.
              doctest
            ];

          shellHook = ''
            export CXXFLAGS=${lib.escapeShellArg cmakeCxxFlags}
            export LDFLAGS=${lib.escapeShellArg cmakeLinkerFlags}
            export ASAN_OPTIONS=detect_leaks=0
          '';

          # Nix assumes fortification by default, but that fails with debug builds.
          # Since this shell is used for development, we disabled fortification. It's
          # still enabled for our release builds in build.nix.
          # https://github.com/NixOS/nixpkgs/issues/18995
          hardeningDisable = ["fortify"];
        };
      };
    };
}
