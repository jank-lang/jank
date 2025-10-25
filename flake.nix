{
  description = "Dev environment for jank";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    self.submodules = true;
  };

  outputs = inputs @ {flake-parts, ...}:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = ["x86_64-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin"];

      perSystem = {
        self',
        pkgs,
        lib,
        ...
      }: let
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

        llvm = pkgs.llvmPackages_git;

      in {
        legacyPackages = pkgs;
        formatter = pkgs.alejandra;

        packages = rec {
          default = jank-release;

          jank-release = pkgs.stdenv.mkDerivation (finalAttrs: {
            pname = "jank";
            version = "git";

            src = lib.cleanSource (lib.fileset.toSource {
              root = ./.;

              fileset = lib.fileset.unions [
                ./.clang-format
                ./compiler+runtime
              ];
            });

            nativeBuildInputs = [
              llvm.libclang.dev
            ] ++ (with pkgs; [
              cmake
              git
              ninja
            ]);

            buildInputs = [
              llvm.libllvm.dev
            ] ++ (with pkgs; [
              bzip2
              libzip
              openssl
              zstd
            ]);

            checkInputs = with pkgs; [
              doctest
              glibcLocales
            ];

            postPatch = ''
              patchShebangs ./compiler+runtime/bin/ar-merge
            '';

            preConfigure = ''
              export JANK_EXTRA_AOT_FLAGS=" \
                  $(cat ${llvm.clang}/nix-support/cc-cflags) \
                  $(cat ${llvm.clang}/nix-support/libc-crt1-cflags) \
                  $(cat ${llvm.clang}/nix-support/cc-ldflags) \
                  $NIX_LDFLAGS \
              "
            '';

            cmakeBuildDir = "./compiler+runtime/build";
            cmakeDir = "..";

            cmakeFlags = [
              "-DCMAKE_C_COMPILER=${llvm.clang}/bin/clang"
              "-DCMAKE_CXX_COMPILER=${llvm.clang}/bin/clang++"

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

        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            stdenv.cc.cc.lib

            ## Required tools.
            cmake
            ninja
            pkg-config
            llvm.clang

            ## Required libs.
            boehmgc
            libzip
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
            export CC=${llvm.clang}/bin/clang
            export CXX=${llvm.clang}/bin/clang++
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
