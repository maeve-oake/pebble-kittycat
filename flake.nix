{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    pebble = {
      url = "github:anna-oake/pebble.nix";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.flake-utils.follows = "flake-utils";
    };
  };

  outputs =
    {
      self,
      pebble,
      nixpkgs,
      flake-utils,
      ...
    }:
    let
      packageJson = builtins.fromJSON (builtins.readFile ./package.json);
      inherit (packageJson) name;
      emulatorTarget = "emery";
    in
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        apps.default = {
          type = "app";
          program = toString (
            pkgs.writeShellScript "install-${name}" ''
              set -euo pipefail
              exec ${pebble.packages.${system}.pebble-tool}/bin/pebble install "${
                self.packages.${system}.default
              }/${name}.pbw" --emulator ${emulatorTarget} --logs "$@"
            ''
          );
        };

        packages.default = pebble.buildPebbleApp.${system} {
          inherit (packageJson) name version;
          src = ./.;
        };

        devShell = pebble.pebbleEnv.${system} {
          inherit emulatorTarget;
        };
      }
    );
}
