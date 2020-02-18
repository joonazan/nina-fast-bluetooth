{ nixpkgs ? import <nixpkgs> {} }:
let
  inherit (nixpkgs) pkgs;

  # esp-idf as of Nov 2019 requires pyparsing < 2.4
  python2 = let
    packageOverrides = self: super: {
            pyparsing = super.pyparsing.overridePythonAttrs( old: rec {
                version = "2.3.1";
                src = super.fetchPypi {
                    pname="pyparsing";
                    inherit version;
                    sha256="66c9268862641abcac4a96ba74506e594c884e3f57690a696d21ad8210ed667a";
                };
        });
    };
    in pkgs.python2.override{inherit packageOverrides; self= python2;};

  esp-idf = pkgs.fetchFromGitHub {
    owner = "espressif";
    repo = "esp-idf";
    rev = "refs/tags/v3.3.1";
    fetchSubmodules = true;
    sha256 = "166b022y87ghav6lp2ky97j8w25ld45vl6kg2k6xvrmzv9lgknm9";
  };
in

pkgs.stdenv.mkDerivation {
  name = "esp-idf-env";

  srcs = [ ./src esp-idf ];
  sourceRoot = "src";

  dontConfigure = true;
  dontBuild = true;
  buildInputs = with pkgs; [
    gawk gperf gettext automake bison flex texinfo help2man libtool autoconf ncurses5 cmake glibcLocales
    (python2.withPackages (ppkgs: with ppkgs; [ pyserial future cryptography setuptools pyelftools pyparsing click ]))
    (pkgs.callPackage ./esp32-toolchain.nix {})
  ];

  IDF_PATH=esp-idf;

  shellHook = ''
    export NIX_CFLAGS_LINK=-lncurses
    export IDF_TOOLS_PATH=$src/tools
    export PATH="$IDF_TOOLS_PATH:$PATH"
  '';
}