{
  description = "csnake";

  inputs.nixpkgs.url = "nixpkgs";

  outputs = { self, nixpkgs }:
    let
      supportedSystems = [ "x86_64-linux" ];
      forAllSystems = f:
        nixpkgs.lib.genAttrs supportedSystems (system: f system);
      nixpkgsFor = forAllSystems (system:
        import nixpkgs {
          inherit system;
          overlays = [ self.overlay ];
        });
    in {
      overlay = final: prev: { csnake = final.callPackage ./. { }; };
      packages =
        forAllSystems (system: { csnake = nixpkgsFor.${system}.csnake; });
      defaultPackage = forAllSystems (system: nixpkgsFor.${system}.csnake);
    };
}
