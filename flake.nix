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
        llvmPackages = pkgs.llvmPackages_23;
        # for cpptrace; versions from cpptrace/cmake/OptionVariables.cmake
        libdwarf-lite-src = pkgs.fetchFromGitHub {
          owner = "jeremy-rifkin";
          repo = "libdwarf-lite";
          rev = "5dfb2cd2aacf2bf473e5bfea79e41289f88b3a5f";
          hash = "sha256-K0vmGJhQgNV3mEShPoIMNnInTkttkj1LD7jByj+/RwA=";
        };
        zstd-src = pkgs.fetchFromGitHub {
          owner = "facebook";
          repo = "zstd";
          rev = "v1.5.7";
          hash = "sha256-tNFWIT9ydfozB8dWcmTMuZLCQmQudTFJIkSr0aG7S44=";
        };
      in {
        formatter = pkgs.alejandra;

        packages = rec {
          default = jank-release;

          jank-release = llvmPackages.stdenv.mkDerivation (finalAttrs: rec {
            pname = "jank";
            version = "git";

            meta = with pkgs.lib; {
              description = "The native Clojure dialect with seamless C++ interop";
              homepage = "https://jank-lang.org";
              license = licenses.mpl20;
              mainProgram = "jank";
            };

            # Add only essential files so that the source hash is consistent.
            src = lib.cleanSource (lib.fileset.toSource {
              root = ./.;
              fileset = lib.fileset.unions [
                ./.clang-format
                ./gdb
                ./compiler+runtime
              ];
            });

            nativeBuildInputs = with pkgs; [
              llvmPackages.clang
              llvmPackages.libclang.dev
              cmake
              git
              ninja
              makeWrapper
            ];

            buildInputs = with pkgs; [
              llvmPackages.libllvm.dev
              bzip2
              openssl
              zstd
              libedit
              libxml2
              boost
            ];

            nativeCheckInputs = with pkgs;
              lib.optionals stdenv.hostPlatform.isLinux [
                glibcLocales
              ];

            postPatch = ''
              patchShebangs ./compiler+runtime/bin/ar-merge
            '';

            # Disable _FORTIFY_SOURCE to prevent linker errors for substituted
            # memcpy/strcpy/etc. symbols.
            #
            # See:  https://github.com/jank-lang/jank/pull/735#issuecomment-4316754811
            #
            # TODO: Figure out how to solve the linker issues without disabling
            # fortification.
            hardeningDisable = ["fortify"];

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

            # jank will execute clang, but via the low-level clang driver API
            # rather than the nix wrapper. We need to manually copy over the
            # flags which would have been filled by the clang wrapper scripts.
            #
            # We pass these on to jank via JANK_EXTRA_FLAGS.
            extraFlags = let
              sources = [
                "${pkgs.binutils}/nix-support/libc-ldflags"
                "${llvmPackages.clang}/nix-support/cc-cflags"
                "${llvmPackages.clang}/nix-support/libc-cflags"
                "${llvmPackages.clang}/nix-support/libc-crt1-cflags"
                "${llvmPackages.clang}/nix-support/libcxx-cxxflags"
                "${llvmPackages.clang}/nix-support/libcxx-ldflags"
                "${llvmPackages.clang}/nix-support/cc-ldflags"
              ];
            in
              lib.strings.concatMapStringsSep " " (
                source: (lib.strings.trim (builtins.readFile source))
              )
              sources;

            postFixup = ''
              wrapProgram "$out/bin/jank" \
                  --argv0 "$out/bin/.jank-wrapped" \
                  --prefix JANK_EXTRA_FLAGS ":" "${extraFlags} $NIX_CFLAGS_COMPILE $NIX_LDFLAGS"
            '';

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
          inputsFrom = [self'.packages.jank-release];

          packages = with pkgs; [
            ## Required tools.
            bubblewrap

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
            leiningen
            expect
            nodejs

            # Examples.
            pkg-config
            glfw3
            libGL
            sqlite

            gnumake
            alsa-lib
            libx11
            libxcursor
            libxi
            libxinerama
            libxrandr
            xorgproto

            ## Book.
            mdbook
            mdbook-mermaid
          ];

          shellHook = ''
            export ASAN_OPTIONS=detect_leaks=0
            export JANK_EXTRA_FLAGS="${self'.packages.jank-release.extraFlags} $NIX_CFLAGS_COMPILE $NIX_LDFLAGS $JANK_EXTRA_FLAGS"
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
