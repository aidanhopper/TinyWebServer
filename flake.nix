{
  description = "A Nix-flake-based C/C++ development environment";

  inputs = {
    nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0.1.*.tar.gz";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system};
      in {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            clang-tools
            cmake
            codespell
            conan
            cppcheck
            doxygen
            gtest
            lcov
            vcpkg
            vcpkg-tool
            inetutils
            clang
          ] ++ (if system == "aarch64-darwin" then [ ] else [ gdb ]);
        };

        packages.default = pkgs.stdenv.mkDerivation {
          name = "server";
          src = ./src;
          buildInputs = with pkgs; [ ];
          buildPhase = ''
            clang main.c -o server
          '';
          installPhase = ''
            mkdir -p $out/bin
            cp server $out/bin
          '';
        };
    });
}

