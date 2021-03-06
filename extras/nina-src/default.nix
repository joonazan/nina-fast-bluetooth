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

in

pkgs.stdenv.mkDerivation {
  name = "esp-idf-env";

  src = ./.;

  dontConfigure = true;
  dontInstall = true;

  buildInputs = with pkgs; [
    gawk gperf gettext automake bison flex texinfo help2man libtool autoconf ncurses5 cmake glibcLocales
    (python2.withPackages (ppkgs: with ppkgs; [ pyserial future cryptography setuptools pyelftools pyparsing click ]))
    (pkgs.callPackage ./esp32-toolchain.nix {})
  ];

  shellHook = ''
    test -e esp-idf || git clone -b v3.3.1 --recursive https://github.com/espressif/esp-idf.git
    export NIX_CFLAGS_LINK=-lncurses
    export IDF_PATH=$(pwd)/esp-idf
    export IDF_TOOLS_PATH=$IDF_PATH/tools
    export PATH="$IDF_TOOLS_PATH:$PATH"
  '';
}