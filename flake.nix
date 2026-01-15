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

        packages = {
          default = (
            pebble.buildPebbleApp.${system} {
              inherit (packageJson) name version;
              type = "watchface";
              src = ./.;

              description = "pretty kitty watchface";

              releaseNotes = ''
                a cat now lives inside your watch.
                good job!
              '';

              sourceUrl = "https://github.com/maeve-oake/pebble-kittycat";

              screenshots = {
                all = [
                  "assets/screenshot.png"
                ];
              };
            }
          );
        };

        devShell = pebble.pebbleEnv.${system} {
          inherit emulatorTarget;
        };
      }
    );
}
