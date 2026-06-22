{
  description = "A Nix flake for logue-sdk development shell with Docker";

  # nixpkgs-unstable is used intentionally; dependencies are pinned via flake.lock.
  # Updates to toolchains are gated by running 'nix flake update' intentionally.
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils }:
    utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            docker
            colima
            emscripten
            clang-tools
            mdformat
            gcc-arm-embedded
          ];

          shellHook = ''
            echo "🐳 Docker development environment loaded!"
            echo "Docker version: $(docker --version 2>/dev/null || echo 'not running/installed')"
            echo "Colima status:  $(colima status 2>&1 || echo 'not running/installed')"
          '';
        };
      });
}
