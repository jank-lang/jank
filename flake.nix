{
  description = "Dev environment for jank";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs = inputs @ {flake-parts, ...}:
    flake-parts.lib.mkFlake {inherit inputs;} {
      systems = ["x86_64-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin"];
      perSystem = {
        pkgs,
        ...
      }: {
        legacyPackages = pkgs;
        formatter = pkgs.alejandra;

        packages.default = pkgs.stdenv.mkDerivation {
          pname = "jank";
          version = "git";
          src = ./.;

          nativeBuildInputs = with pkgs; [
            git
            cmake
            ninja
            (pkgs.callPackage ./llvm.nix {})
          ];
          buildInputs = with pkgs; [libzip openssl];

          postPatch = ''
            patchShebangs ./compiler+runtime/bin/ar-merge
          '';

          cmakeBuildDir = "./compiler+runtime/build";
          cmakeDir = "..";
          cmakeFlags = [
            "-DCMAKE_C_COMPILER=clang"
            "-DCMAKE_CXX_COMPILER=clang++"
            # TODO: Updating RPATHs during install causes the step to fail as it
            # tries to rewrite non-existent RPATHs like /lib. Needs more
            # investigation.
            "-DCMAKE_SKIP_RPATH=ON"
            # TODO: To use libdwarf (recommended) we need to build the custom
            # patched version available from CppTrace. Normally this is done via
            # FetchContent, but that's not allowed in a nix build. As a
            # workaround we use libdl which is always available.
            "-DCPPTRACE_GET_SYMBOLS_WITH_LIBDL=ON"
            "-Djank_unity_build=on"
          ];
        };

        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            stdenv.cc.cc.lib

            ## Required tools.
            cmake
            ninja
            pkg-config
            (pkgs.callPackage ./llvm.nix { })

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

          shellHook =
          ''
          export CC=clang
          export CXX=clang++
          export ASAN_OPTIONS=detect_leaks=0
          '';

          # Nix assumes fortification by default, but that fails with debug builds.
          # Since this shell is used for development, we disabled fortification. It's
          # still enabled for our release builds in build.nix.
          # https://github.com/NixOS/nixpkgs/issues/18995
          hardeningDisable = [ "fortify" ];
        };
      };
    };
}
