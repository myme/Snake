{
  pkgs ? import <nixpkgs> {},
  stdenv ? pkgs.stdenv,
  ncurses ? pkgs.ncurses,
}:

stdenv.mkDerivation {
  name = "csnake";
  version = "0.1.0";
  src = ./.;
  buildInputs = [ ncurses ];
}
